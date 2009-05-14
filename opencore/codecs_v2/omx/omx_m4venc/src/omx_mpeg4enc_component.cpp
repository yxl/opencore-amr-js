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

#include "omx_mpeg4enc_component.h"

#if PROXY_INTERFACE
#include "omx_proxy_interface.h"
#endif



// This function is called by OMX_GetHandle and it creates an instance of the mpeg4 component AO
OMX_ERRORTYPE Mpeg4EncOmxComponentFactory(OMX_OUT OMX_HANDLETYPE* pHandle, OMX_IN OMX_PTR pAppData, OMX_PTR pProxy, OMX_STRING aOmxLibName, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount)
{
    OSCL_UNUSED_ARG(aOmxLibName);
    OSCL_UNUSED_ARG(aOmxLib);
    OSCL_UNUSED_ARG(aOsclUuid);
    OSCL_UNUSED_ARG(aRefCount);


    OmxComponentMpeg4EncAO* pOpenmaxAOType;
    OMX_ERRORTYPE Status;

    // move InitMpeg4OmxComponentFields content to actual constructor

    pOpenmaxAOType = (OmxComponentMpeg4EncAO*) OSCL_NEW(OmxComponentMpeg4EncAO, ());

    if (NULL == pOpenmaxAOType)
    {
        return OMX_ErrorInsufficientResources;
    }

    // set encoding mode to H263
    pOpenmaxAOType->SetEncoderMode(MODE_MPEG4);

    //Call the construct component to initialize OMX types
    Status = pOpenmaxAOType->ConstructComponent(pAppData, pProxy);

    *pHandle = pOpenmaxAOType->GetOmxHandle();

    return Status;
}


// This function is called by OMX_FreeHandle when component AO needs to be destroyed
OMX_ERRORTYPE Mpeg4EncOmxComponentDestructor(OMX_IN OMX_HANDLETYPE pHandle, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount)
{
    OSCL_UNUSED_ARG(aOmxLib);
    OSCL_UNUSED_ARG(aOsclUuid);
    OSCL_UNUSED_ARG(aRefCount);

    // get pointer to component AO
    OmxComponentMpeg4EncAO* pOpenmaxAOType = (OmxComponentMpeg4EncAO*)((OMX_COMPONENTTYPE*)pHandle)->pComponentPrivate;

    // clean up decoder, OMX component stuff
    pOpenmaxAOType->DestroyComponent();

    // destroy the AO class
    OSCL_DELETE(pOpenmaxAOType);

    return OMX_ErrorNone;
}


// This function is called by OMX_GetHandle and it creates an instance of the h263 component AO
OMX_ERRORTYPE H263EncOmxComponentFactory(OMX_OUT OMX_HANDLETYPE* pHandle, OMX_IN  OMX_PTR pAppData, OMX_PTR pProxy, OMX_STRING aOmxLibName, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount)
{
    OSCL_UNUSED_ARG(aOmxLibName);
    OSCL_UNUSED_ARG(aOmxLib);
    OSCL_UNUSED_ARG(aOsclUuid);
    OSCL_UNUSED_ARG(aRefCount);


    OmxComponentMpeg4EncAO* pOpenmaxAOType;
    OMX_ERRORTYPE Status;

    // move InitMpeg4OmxComponentFields content to actual constructor

    pOpenmaxAOType = (OmxComponentMpeg4EncAO*) OSCL_NEW(OmxComponentMpeg4EncAO, ());

    if (NULL == pOpenmaxAOType)
    {
        return OMX_ErrorInsufficientResources;
    }

    // set encoding mode to H263
    pOpenmaxAOType->SetEncoderMode(MODE_H263);

    //Call the construct component to initialize OMX types
    Status = pOpenmaxAOType->ConstructComponent(pAppData, pProxy);

    *pHandle = pOpenmaxAOType->GetOmxHandle();

    return Status;
    ///////////////////////////////////////////////////////////////////////////////////////
}


// This function is called by OMX_FreeHandle when component AO needs to be destroyed
OMX_ERRORTYPE H263EncOmxComponentDestructor(OMX_IN OMX_HANDLETYPE pHandle, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount)
{
    OSCL_UNUSED_ARG(aOmxLib);
    OSCL_UNUSED_ARG(aOsclUuid);
    OSCL_UNUSED_ARG(aRefCount);

    // get pointer to component AO
    OmxComponentMpeg4EncAO* pOpenmaxAOType = (OmxComponentMpeg4EncAO*)((OMX_COMPONENTTYPE*)pHandle)->pComponentPrivate;

    // clean up decoder, OMX component stuff
    pOpenmaxAOType->DestroyComponent();

    // destroy the AO class
    OSCL_DELETE(pOpenmaxAOType);

    return OMX_ErrorNone;
}

#if (DYNAMIC_LOAD_OMX_M4VENC_COMPONENT || DYNAMIC_LOAD_OMX_H263ENC_COMPONENT)
class Mpeg4H263EncOmxSharedLibraryInterface:  public OsclSharedLibraryInterface,
            public OmxSharedLibraryInterface

{
    public:
        static Mpeg4H263EncOmxSharedLibraryInterface *Instance()
        {
            static Mpeg4H263EncOmxSharedLibraryInterface omxinterface;
            return &omxinterface;
        };

        OsclAny *QueryOmxComponentInterface(const OsclUuid& aOmxTypeId, const OsclUuid& aInterfaceId)
        {
            if (PV_OMX_M4VENC_UUID == aOmxTypeId)
            {
                if (PV_OMX_CREATE_INTERFACE == aInterfaceId)
                {
                    return ((OsclAny*)(&Mpeg4EncOmxComponentFactory));
                }
                else if (PV_OMX_DESTROY_INTERFACE == aInterfaceId)
                {
                    return ((OsclAny*)(&Mpeg4EncOmxComponentDestructor));
                }
            }
            else if (PV_OMX_H263ENC_UUID == aOmxTypeId)
            {
                if (PV_OMX_CREATE_INTERFACE == aInterfaceId)
                {
                    return ((OsclAny*)(&H263EncOmxComponentFactory));
                }
                else if (PV_OMX_DESTROY_INTERFACE == aInterfaceId)
                {
                    return ((OsclAny*)(&H263EncOmxComponentDestructor));
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
        Mpeg4H263EncOmxSharedLibraryInterface() {};
};

// function to obtain the interface object from the shared library
extern "C"
{
    OSCL_EXPORT_REF OsclAny* PVGetInterface()
    {
        return Mpeg4H263EncOmxSharedLibraryInterface::Instance();
    }
}

#endif

void OmxComponentMpeg4EncAO::SetEncoderMode(OMX_S32 aMode)
{
    iEncMode = aMode;
}



OMX_ERRORTYPE OmxComponentMpeg4EncAO::ConstructComponent(OMX_PTR pAppData, OMX_PTR pProxy)
{
    ComponentPortType *pInPort, *pOutPort;
    OMX_ERRORTYPE Status;

    iNumPorts = 2;
    iCompressedFormatPortNum = OMX_PORT_OUTPUTPORT_INDEX;
    iOmxComponent.nSize = sizeof(OMX_COMPONENTTYPE);
    iOmxComponent.pComponentPrivate = (OMX_PTR) this;  // pComponentPrivate points to THIS component AO class
    ipComponentProxy = pProxy;
    iOmxComponent.pApplicationPrivate = pAppData; // init the App data


#if PROXY_INTERFACE
    iPVCapabilityFlags.iIsOMXComponentMultiThreaded = OMX_TRUE;

    iOmxComponent.SendCommand = OmxComponentMpeg4EncAO::BaseComponentProxySendCommand;
    iOmxComponent.GetParameter = OmxComponentMpeg4EncAO::BaseComponentProxyGetParameter;
    iOmxComponent.SetParameter = OmxComponentMpeg4EncAO::BaseComponentProxySetParameter;
    iOmxComponent.GetConfig = OmxComponentMpeg4EncAO::BaseComponentProxyGetConfig;
    iOmxComponent.SetConfig = OmxComponentMpeg4EncAO::BaseComponentProxySetConfig;
    iOmxComponent.GetExtensionIndex = OmxComponentMpeg4EncAO::BaseComponentProxyGetExtensionIndex;
    iOmxComponent.GetState = OmxComponentMpeg4EncAO::BaseComponentProxyGetState;
    iOmxComponent.UseBuffer = OmxComponentMpeg4EncAO::BaseComponentProxyUseBuffer;
    iOmxComponent.AllocateBuffer = OmxComponentMpeg4EncAO::BaseComponentProxyAllocateBuffer;
    iOmxComponent.FreeBuffer = OmxComponentMpeg4EncAO::BaseComponentProxyFreeBuffer;
    iOmxComponent.EmptyThisBuffer = OmxComponentMpeg4EncAO::BaseComponentProxyEmptyThisBuffer;
    iOmxComponent.FillThisBuffer = OmxComponentMpeg4EncAO::BaseComponentProxyFillThisBuffer;

#else
    iPVCapabilityFlags.iIsOMXComponentMultiThreaded = OMX_FALSE;

    iOmxComponent.SendCommand = OmxComponentMpeg4EncAO::BaseComponentSendCommand;
    iOmxComponent.GetParameter = OmxComponentMpeg4EncAO::BaseComponentGetParameter;
    iOmxComponent.SetParameter = OmxComponentMpeg4EncAO::BaseComponentSetParameter;
    iOmxComponent.GetConfig = OmxComponentMpeg4EncAO::BaseComponentGetConfig;
    iOmxComponent.SetConfig = OmxComponentMpeg4EncAO::BaseComponentSetConfig;
    iOmxComponent.GetExtensionIndex = OmxComponentMpeg4EncAO::BaseComponentGetExtensionIndex;
    iOmxComponent.GetState = OmxComponentMpeg4EncAO::BaseComponentGetState;
    iOmxComponent.UseBuffer = OmxComponentMpeg4EncAO::BaseComponentUseBuffer;
    iOmxComponent.AllocateBuffer = OmxComponentMpeg4EncAO::BaseComponentAllocateBuffer;
    iOmxComponent.FreeBuffer = OmxComponentMpeg4EncAO::BaseComponentFreeBuffer;
    iOmxComponent.EmptyThisBuffer = OmxComponentMpeg4EncAO::BaseComponentEmptyThisBuffer;
    iOmxComponent.FillThisBuffer = OmxComponentMpeg4EncAO::BaseComponentFillThisBuffer;
#endif

    iOmxComponent.SetCallbacks = OmxComponentMpeg4EncAO::BaseComponentSetCallbacks;
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

    /** Domain specific section for input raw port */ //OMX_PARAM_PORTDEFINITIONTYPE
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.nPortIndex = OMX_PORT_INPUTPORT_INDEX;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.eDomain = OMX_PortDomainVideo;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.format.video.cMIMEType = (OMX_STRING)"raw";
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.format.video.eCompressionFormat = OMX_VIDEO_CodingUnused;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.format.video.pNativeRender = 0;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.format.video.bFlagErrorConcealment = OMX_FALSE;

    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.format.video.eColorFormat = OMX_COLOR_FormatYUV420Planar;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.format.video.nFrameWidth = 176;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.format.video.nFrameHeight = 144;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.format.video.nBitrate = 64000;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.format.video.xFramerate = (15 << 16);
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.eDir = OMX_DirInput;
    //Set to a default value, will change later during setparameter call
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.nBufferCountActual = NUMBER_INPUT_BUFFER_MP4ENC;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.nBufferCountMin = 1;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.nBufferSize = INPUT_BUFFER_SIZE_MP4ENC;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.bEnabled = OMX_TRUE;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.bPopulated = OMX_FALSE;


    /** Domain specific section for output mpeg4/h263 port */
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.nPortIndex = OMX_PORT_OUTPUTPORT_INDEX;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.eDomain = OMX_PortDomainVideo;
    if (MODE_MPEG4 == iEncMode)
    {
        ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.format.video.cMIMEType = (OMX_STRING)"video/mpeg4";
        ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.format.video.eCompressionFormat = OMX_VIDEO_CodingMPEG4;

    }
    else if (MODE_H263 == iEncMode)
    {
        ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.format.video.cMIMEType = (OMX_STRING)"video/h263";
        ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.format.video.eCompressionFormat = OMX_VIDEO_CodingH263;
    }

    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.format.video.pNativeRender = 0;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.format.video.bFlagErrorConcealment = OMX_FALSE;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.format.video.eColorFormat = OMX_COLOR_FormatUnused;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.format.video.nFrameWidth = 176;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.format.video.nFrameHeight = 144;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.format.video.nBitrate = 64000;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.format.video.xFramerate = (15 << 16);
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.eDir = OMX_DirOutput;
    //Set to a default value, will change later during setparameter call
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.nBufferCountActual = NUMBER_OUTPUT_BUFFER_MP4ENC;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.nBufferCountMin = 1;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.nBufferSize = OUTPUT_BUFFER_SIZE_MP4ENC;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.bEnabled = OMX_TRUE;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.bPopulated = OMX_FALSE;


    //OMX_VIDEO_PARAM_MPEG4TYPE
    //Default values for mpeg4 video output param port
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->VideoMpeg4.nPortIndex = OMX_PORT_OUTPUTPORT_INDEX;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->VideoMpeg4.eProfile = OMX_VIDEO_MPEG4ProfileCore;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->VideoMpeg4.eLevel = OMX_VIDEO_MPEG4Level2;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->VideoMpeg4.nPFrames = 10;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->VideoMpeg4.nBFrames = 0;		//No support for B frames
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->VideoMpeg4.nMaxPacketSize = 256;	//Default value
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->VideoMpeg4.nAllowedPictureTypes = OMX_VIDEO_PictureTypeI | OMX_VIDEO_PictureTypeP;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->VideoMpeg4.bGov = OMX_FALSE;

    //This will be an additional structure maintained on output port if the encoder is H.263
    if (MODE_H263 == iEncMode)
    {
        ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->VideoH263.nPortIndex = OMX_PORT_OUTPUTPORT_INDEX;
        ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->VideoH263.eProfile = OMX_VIDEO_H263ProfileBaseline;
        ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->VideoH263.eLevel = OMX_VIDEO_H263Level45;
        ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->VideoH263.bPLUSPTYPEAllowed = OMX_FALSE;
        ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->VideoH263.nAllowedPictureTypes = OMX_VIDEO_PictureTypeI | OMX_VIDEO_PictureTypeP;
        ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->VideoH263.bForceRoundingTypeToZero = OMX_TRUE;
        ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->VideoH263.nPictureHeaderRepetition = 0;
        ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->VideoH263.nGOBHeaderInterval = 0;
    }


    if (MODE_MPEG4 == iEncMode)
    {
        //OMX_VIDEO_PARAM_PROFILELEVELTYPE structure
        ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->ProfileLevel.nPortIndex = OMX_PORT_OUTPUTPORT_INDEX;
        ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->ProfileLevel.nProfileIndex = 0;
        ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->ProfileLevel.eProfile = OMX_VIDEO_MPEG4ProfileCore;
        ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->ProfileLevel.eLevel = OMX_VIDEO_MPEG4Level2;
    }
    else if (MODE_H263 == iEncMode)
    {
        ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->ProfileLevel.nPortIndex = OMX_PORT_OUTPUTPORT_INDEX;
        ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->ProfileLevel.nProfileIndex = 0;
        ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->ProfileLevel.eProfile = OMX_VIDEO_H263ProfileBaseline;
        ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->ProfileLevel.eLevel = OMX_VIDEO_H263Level45;
    }


    iPortTypesParam.nPorts = 2;
    iPortTypesParam.nStartPortNumber = 0;

    pInPort = (ComponentPortType*) ipPorts[OMX_PORT_INPUTPORT_INDEX];
    pOutPort = (ComponentPortType*) ipPorts[OMX_PORT_OUTPUTPORT_INDEX];

    pInPort->ActualNumPortFormatsSupported = 4;

    //OMX_VIDEO_PARAM_PORTFORMATTYPE INPUT PORT SETTINGS
    //On input port for index 0
    SetHeader(&pInPort->VideoParam[0], sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
    pInPort->VideoParam[0].nPortIndex = OMX_PORT_INPUTPORT_INDEX;
    pInPort->VideoParam[0].nIndex = 0;
    pInPort->VideoParam[0].eCompressionFormat = OMX_VIDEO_CodingUnused;
    pInPort->VideoParam[0].eColorFormat = OMX_COLOR_FormatYUV420Planar;
    pInPort->VideoParam[0].xFramerate = (15 << 16);

    //On input port for index 1
    SetHeader(&pInPort->VideoParam[1], sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
    pInPort->VideoParam[1].nPortIndex = OMX_PORT_INPUTPORT_INDEX;
    pInPort->VideoParam[1].nIndex = 1;
    pInPort->VideoParam[1].eCompressionFormat = OMX_VIDEO_CodingUnused;
    pInPort->VideoParam[1].eColorFormat = OMX_COLOR_Format24bitRGB888;
    pInPort->VideoParam[1].xFramerate = (15 << 16);

    //On input port for index 2
    SetHeader(&pInPort->VideoParam[2], sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
    pInPort->VideoParam[2].nPortIndex = OMX_PORT_INPUTPORT_INDEX;
    pInPort->VideoParam[2].nIndex = 2;
    pInPort->VideoParam[2].eCompressionFormat = OMX_VIDEO_CodingUnused;
    pInPort->VideoParam[2].eColorFormat = OMX_COLOR_Format12bitRGB444;
    pInPort->VideoParam[2].xFramerate = (15 << 16);

    //On input port for index 3
    SetHeader(&pInPort->VideoParam[3], sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
    pInPort->VideoParam[3].nPortIndex = OMX_PORT_INPUTPORT_INDEX;
    pInPort->VideoParam[3].nIndex = 3;
    pInPort->VideoParam[3].eCompressionFormat = OMX_VIDEO_CodingUnused;
    pInPort->VideoParam[3].eColorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
    pInPort->VideoParam[3].xFramerate = (15 << 16);


    pOutPort->ActualNumPortFormatsSupported = 1;

    //OMX_VIDEO_PARAM_PORTFORMATTYPE OUTPUT PORT SETTINGS
    //On output port for index 0
    SetHeader(&pOutPort->VideoParam[0], sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
    pOutPort->VideoParam[0].nPortIndex = OMX_PORT_OUTPUTPORT_INDEX;
    pOutPort->VideoParam[0].nIndex = 0;
    pOutPort->VideoParam[0].eColorFormat = OMX_COLOR_FormatUnused;
    pOutPort->VideoParam[0].xFramerate = (15 << 16);
    if (iEncMode == MODE_MPEG4)
    {
        pOutPort->VideoParam[0].eCompressionFormat = OMX_VIDEO_CodingMPEG4;

    }
    else if (iEncMode == MODE_H263)
    {
        pOutPort->VideoParam[0].eCompressionFormat = OMX_VIDEO_CodingH263;
    }


    //OMX_CONFIG_ROTATIONTYPE SETTINGS ON INPUT PORT
    SetHeader(&pInPort->VideoOrientationType, sizeof(OMX_CONFIG_ROTATIONTYPE));
    pInPort->VideoOrientationType.nPortIndex = OMX_PORT_INPUTPORT_INDEX;
    pInPort->VideoOrientationType.nRotation = -1;  //For all the YUV formats that are other than RGB


    //OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE settings of output port
    oscl_memset(&pOutPort->VideoErrorCorrection, 0, sizeof(OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE));
    SetHeader(&pOutPort->VideoErrorCorrection, sizeof(OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE));
    pOutPort->VideoErrorCorrection.nPortIndex = OMX_PORT_OUTPUTPORT_INDEX;
    pOutPort->VideoErrorCorrection.bEnableDataPartitioning = OMX_FALSE;	//As in node default is h263


    //OMX_VIDEO_PARAM_BITRATETYPE settings of output port
    SetHeader(&pOutPort->VideoRateType, sizeof(OMX_VIDEO_PARAM_BITRATETYPE));
    pOutPort->VideoRateType.nPortIndex = OMX_PORT_OUTPUTPORT_INDEX;
    pOutPort->VideoRateType.eControlRate = OMX_Video_ControlRateConstant;
    pOutPort->VideoRateType.nTargetBitrate = 64000;


    //OMX_CONFIG_FRAMERATETYPE default seetings (specified in khronos conformance test)
    SetHeader(&pOutPort->VideoConfigFrameRateType, sizeof(OMX_CONFIG_FRAMERATETYPE));
    pOutPort->VideoConfigFrameRateType.nPortIndex = OMX_PORT_OUTPUTPORT_INDEX;
    pOutPort->VideoConfigFrameRateType.xEncodeFramerate = (15 << 16);

    //OMX_VIDEO_CONFIG_BITRATETYPE default settings (specified in khronos conformance test)
    SetHeader(&pOutPort->VideoConfigBitRateType, sizeof(OMX_VIDEO_CONFIG_BITRATETYPE));
    pOutPort->VideoConfigBitRateType.nPortIndex = OMX_PORT_OUTPUTPORT_INDEX;
    pOutPort->VideoConfigBitRateType.nEncodeBitrate = 64000;


    //OMX_VIDEO_PARAM_QUANTIZATIONTYPE settings of output port
    SetHeader(&pOutPort->VideoQuantType, sizeof(OMX_VIDEO_PARAM_QUANTIZATIONTYPE));
    pOutPort->VideoQuantType.nPortIndex = OMX_PORT_OUTPUTPORT_INDEX;
    pOutPort->VideoQuantType.nQpI = 15;
    pOutPort->VideoQuantType.nQpP = 12;
    pOutPort->VideoQuantType.nQpB = 12;


    //OMX_VIDEO_PARAM_VBSMCTYPE settings of output port
    oscl_memset(&pOutPort->VideoBlockMotionSize, 0, sizeof(OMX_VIDEO_PARAM_VBSMCTYPE));
    SetHeader(&pOutPort->VideoBlockMotionSize, sizeof(OMX_VIDEO_PARAM_VBSMCTYPE));
    pOutPort->VideoBlockMotionSize.nPortIndex = OMX_PORT_OUTPUTPORT_INDEX;
    pOutPort->VideoBlockMotionSize.b16x16 = OMX_TRUE;


    //OMX_VIDEO_PARAM_MOTIONVECTORTYPE settings of output port
    oscl_memset(&pOutPort->VideoMotionVector, 0, sizeof(OMX_VIDEO_PARAM_MOTIONVECTORTYPE));
    SetHeader(&pOutPort->VideoMotionVector, sizeof(OMX_VIDEO_PARAM_MOTIONVECTORTYPE));
    pOutPort->VideoMotionVector.nPortIndex = OMX_PORT_OUTPUTPORT_INDEX;
    pOutPort->VideoMotionVector.eAccuracy = OMX_Video_MotionVectorHalfPel;
    pOutPort->VideoMotionVector.bUnrestrictedMVs = OMX_TRUE;
    pOutPort->VideoMotionVector.sXSearchRange = 16;
    pOutPort->VideoMotionVector.sYSearchRange = 16;


    //OMX_VIDEO_PARAM_INTRAREFRESHTYPE settings of output port
    oscl_memset(&pOutPort->VideoIntraRefresh, 0, sizeof(OMX_VIDEO_PARAM_INTRAREFRESHTYPE));
    SetHeader(&pOutPort->VideoIntraRefresh, sizeof(OMX_VIDEO_PARAM_INTRAREFRESHTYPE));
    pOutPort->VideoIntraRefresh.nPortIndex = OMX_PORT_OUTPUTPORT_INDEX;
    pOutPort->VideoIntraRefresh.eRefreshMode = OMX_VIDEO_IntraRefreshCyclic;
    pOutPort->VideoIntraRefresh.nCirMBs = 0;


    //OMX_CONFIG_INTRAREFRESHVOPTYPE settings of output port
    oscl_memset(&pOutPort->VideoIFrame, 0, sizeof(OMX_CONFIG_INTRAREFRESHVOPTYPE));
    SetHeader(&pOutPort->VideoIFrame, sizeof(OMX_CONFIG_INTRAREFRESHVOPTYPE));
    pOutPort->VideoIFrame.nPortIndex = OMX_PORT_OUTPUTPORT_INDEX;
    pOutPort->VideoIFrame.IntraRefreshVOP = OMX_FALSE;


    //Construct the encoder object
    if (ipMpegEncoderObject)
    {
        OSCL_DELETE(ipMpegEncoderObject);
        ipMpegEncoderObject = NULL;
    }

    ipMpegEncoderObject = OSCL_NEW(Mpeg4Encoder_OMX, ());

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

OMX_ERRORTYPE OmxComponentMpeg4EncAO::DestroyComponent()
{
    if (OMX_FALSE != iIsInit)
    {
        ComponentDeInit();
    }

    //Destroy the base class now
    DestroyBaseComponent();

    if (ipMpegEncoderObject)
    {
        OSCL_DELETE(ipMpegEncoderObject);
        ipMpegEncoderObject = NULL;
    }

    if (ipAppPriv)
    {
        ipAppPriv->CompHandle = NULL;

        oscl_free(ipAppPriv);
        ipAppPriv = NULL;
    }

    return OMX_ErrorNone;
}


void OmxComponentMpeg4EncAO::ProcessData()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentMpeg4EncAO : ProcessData IN"));

    QueueType* pInputQueue = ipPorts[OMX_PORT_INPUTPORT_INDEX]->pBufferQueue;
    QueueType* pOutputQueue = ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->pBufferQueue;

    ComponentPortType*	pInPort = ipPorts[OMX_PORT_INPUTPORT_INDEX];
    ComponentPortType*	pOutPort = ipPorts[OMX_PORT_OUTPUTPORT_INDEX];

    OMX_U8*					pOutBuffer;
    OMX_U32					OutputLength;
    OMX_BOOL				EncodeReturn = OMX_FALSE;
    OMX_COMPONENTTYPE*		pHandle = &iOmxComponent;

    if ((!iIsInputBufferEnded) || (iEndofStream))
    {
        //Check whether prev output bufer has been released or not
        if (OMX_TRUE == iNewOutBufRequired)
        {
            //Check whether a new output buffer is available or not

            if (0 == (GetQueueNumElem(pOutputQueue)))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentMpeg4EncAO : ProcessData OUT output buffer unavailable"));
                return;
            }

            ipOutputBuffer = (OMX_BUFFERHEADERTYPE*) DeQueue(pOutputQueue);

            OSCL_ASSERT(NULL != ipOutputBuffer);
            if (ipOutputBuffer == NULL)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentMpeg4EncAO : ProcessData OUT ERR output buffer cannot be dequeued"));
                return;
            }

            ipOutputBuffer->nFilledLen = 0;
            iNewOutBufRequired = OMX_FALSE;


            /* If some output data was left to be send from the last processing due to
             * unavailability of required number of output buffers,
             * copy it now and send back before processing new input frame */
            if (iInternalOutBufFilledLen > 0)
            {
                if (OMX_FALSE == CopyDataToOutputBuffer())
                {
                    //We fell short of output buffers, exit now and wait for some more buffers to get queued
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentMpeg4EncAO : ProcessData OUT output buffer unavailable"));
                    return;
                }
                else
                {
                    //Attach the end of frame flag while sending out the last piece of output buffer
                    ipOutputBuffer->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;
                    if (OMX_TRUE == iSyncFlag)
                    {
                        ipOutputBuffer->nFlags |= OMX_BUFFERFLAG_SYNCFRAME;
                        iSyncFlag = OMX_FALSE;
                    }
                    ReturnOutputBuffer(ipOutputBuffer, pOutPort);

                    //Dequeue new output buffer to continue encoding the next frame
                    if (0 == (GetQueueNumElem(pOutputQueue)))
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentMpeg4EncAO : ProcessData OUT output buffer unavailable"));
                        return;
                    }

                    ipOutputBuffer = (OMX_BUFFERHEADERTYPE*) DeQueue(pOutputQueue);

                    OSCL_ASSERT(NULL != ipOutputBuffer);
                    if (ipOutputBuffer == NULL)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentMpeg4EncAO : ProcessData OUT ERR output buffer cannot be dequeued"));
                        return;
                    }

                    ipOutputBuffer->nFilledLen = 0;
                    iNewOutBufRequired = OMX_FALSE;
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

        //Call the encoder only if there is some data to encode
        if (iInputCurrLength > 0)
        {
            pOutBuffer = ipOutputBuffer->pBuffer;
            OutputLength = ipOutputBuffer->nAllocLen;

            //Output buffer is passed as a short pointer
            EncodeReturn = ipMpegEncoderObject->Mp4EncodeVideo(pOutBuffer,
                           &OutputLength,
                           &iBufferOverRun,
                           &ipInternalOutBuffer,
                           ipFrameDecodeBuffer,
                           iInputCurrLength,
                           iFrameTimestamp,
                           &iOutputTimeStamp,
                           &iSyncFlag);


            //Chk whether output data has been generated or not
            if (OutputLength > 0)
            {
                //offset not required in our case, set it to zero
                ipOutputBuffer->nOffset = 0;
                ipOutputBuffer->nTimeStamp = iOutputTimeStamp;

                if (OMX_FALSE == iBufferOverRun)
                {
                    //No internal buffer is maintained
                    ipOutputBuffer->nFilledLen = OutputLength;
                }
                else
                {
                    iInternalOutBufFilledLen = OutputLength;
                    iBufferOverRun = OMX_FALSE;
                    CopyDataToOutputBuffer();

                }	//else loop of if (OMX_FALSE == iMantainOutInternalBuffer)
            }	//if (OutputLength > 0)	 loop


            //If encoder returned error in case of frame skip/corrupt frame, report it to the client via a callback
            if ((OMX_FALSE == EncodeReturn) && (OMX_FALSE == iEndofStream))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentMpeg4EncAO : Frame skipped, ProcessData ErrorStreamCorrupt callback send"));

                (*(ipCallbacks->EventHandler))
                (pHandle,
                 iCallbackData,
                 OMX_EventError,
                 OMX_ErrorStreamCorrupt,
                 0,
                 NULL);
            }


            //For the first time, encoder returns the volheader in output buffer and input remains unconsumed
            //so do not return the input buffer yet
            if (0 != iFrameCount)
            {
                //Input bytes consumed now, return the buffer
                ipInputBuffer->nFilledLen = 0;
                ReturnInputBuffer(ipInputBuffer, pInPort);
                ipInputBuffer = NULL;

                iIsInputBufferEnded = OMX_TRUE;
                iInputCurrLength = 0;
            }

            iFrameCount++;
        }


        /* If EOS flag has come from the client & there are no more
         * input buffers to decode, send the callback to the client
         */
        if (OMX_TRUE == iEndofStream)
        {
            if (((0 == iInputCurrLength) || (OMX_FALSE == EncodeReturn)) &&
                    (0 == iInternalOutBufFilledLen))
            {

                (*(ipCallbacks->EventHandler))
                (pHandle,
                 iCallbackData,
                 OMX_EventBufferFlag,
                 1,
                 OMX_BUFFERFLAG_EOS,
                 NULL);
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentMpeg4EncAO : ProcessData EOS callback sent"));


                //Mark this flag false once the callback has been send back
                iEndofStream = OMX_FALSE;

                ipOutputBuffer->nFlags |= OMX_BUFFERFLAG_EOS;
                if (OMX_TRUE == iSyncFlag)
                {
                    ipOutputBuffer->nFlags |= OMX_BUFFERFLAG_SYNCFRAME;
                    iSyncFlag = OMX_FALSE;
                }
                ReturnOutputBuffer(ipOutputBuffer, pOutPort);

                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentMpeg4EncAO : ProcessData OUT"));
                return;
            }

        }

        //Send the output buffer back after decode
        if ((ipOutputBuffer->nFilledLen > 0) && (OMX_FALSE == iNewOutBufRequired))
        {
            //Attach the end of frame flag while sending out the last piece of output buffer
            ipOutputBuffer->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;
            if (OMX_TRUE == iSyncFlag)
            {
                ipOutputBuffer->nFlags |= OMX_BUFFERFLAG_SYNCFRAME;
                iSyncFlag = OMX_FALSE;
            }
            ReturnOutputBuffer(ipOutputBuffer, pOutPort);
        }


        /* If there is some more processing left with current buffers, re-schedule the AO
         * Do not go for more than one round of processing at a time.
         * This may block the AO longer than required.
         */
        if ((iInputCurrLength != 0 || GetQueueNumElem(pInputQueue) > 0)
                && (GetQueueNumElem(pOutputQueue) > 0))
        {
            RunIfNotReady();
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentMpeg4EncAO : ProcessData OUT"));
}


OMX_BOOL OmxComponentMpeg4EncAO::CopyDataToOutputBuffer()
{
    ComponentPortType*	pOutPort = ipPorts[OMX_PORT_OUTPUTPORT_INDEX];
    QueueType* pOutputQueue = ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->pBufferQueue;

    while (iInternalOutBufFilledLen > 0)
    {
        if (ipOutputBuffer->nAllocLen >= iInternalOutBufFilledLen)
        {
            //Pack the whole  data into the output buffer Alloc length data in one buffer and return it
            oscl_memcpy(ipOutputBuffer->pBuffer, ipInternalOutBuffer, iInternalOutBufFilledLen);
            ipOutputBuffer->nFilledLen = iInternalOutBufFilledLen;

        }
        else
        {
            oscl_memcpy(ipOutputBuffer->pBuffer, ipInternalOutBuffer, ipOutputBuffer->nAllocLen);
            ipOutputBuffer->nFilledLen = ipOutputBuffer->nAllocLen;
        }

        iInternalOutBufFilledLen -= ipOutputBuffer->nFilledLen;
        ipInternalOutBuffer += ipOutputBuffer->nFilledLen;


        if (0 != iInternalOutBufFilledLen)
        {
            //Mark sync flag in each piece of partial output buffer
            if (OMX_TRUE == iSyncFlag)
            {
                ipOutputBuffer->nFlags |= OMX_BUFFERFLAG_SYNCFRAME;
            }
            //Return the partial output buffer and try to fetch a new output buffer for filling the remaining data
            ReturnOutputBuffer(ipOutputBuffer, pOutPort);

            //Check whether a new output buffer is available or not
            if (0 == (GetQueueNumElem(pOutputQueue)))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentMpeg4EncAO : CopyDataToOutputBuffer OUT output buffer unavailable"));
                return OMX_FALSE;
            }

            ipOutputBuffer = (OMX_BUFFERHEADERTYPE*) DeQueue(pOutputQueue);

            OSCL_ASSERT(NULL != ipOutputBuffer);
            if (ipOutputBuffer == NULL)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentMpeg4EncAO : CopyDataToOutputBuffer  OUT ERR output buffer cannot be dequeued"));
                return OMX_FALSE;
            }

            ipOutputBuffer->nFilledLen = 0;
            ipOutputBuffer->nTimeStamp = iOutputTimeStamp;
            ipOutputBuffer->nOffset = 0;
            iNewOutBufRequired = OMX_FALSE;
        }
    }	//while (iInternalOutBufFilledLen > 0)

    return OMX_TRUE;

}


//Not implemented & supported in case of base profile components

void OmxComponentMpeg4EncAO::ComponentGetRolesOfComponent(OMX_STRING* aRoleString)
{
    *aRoleString = (OMX_STRING)"video_encoder.mpeg4";
}


//Component constructor
OmxComponentMpeg4EncAO::OmxComponentMpeg4EncAO()
{
    ipMpegEncoderObject = NULL;
    iEncMode = MODE_H263;
    //iMantainOutInternalBuffer = OMX_FALSE;
    ipInternalOutBuffer = NULL;
    iInternalOutBufFilledLen = 0;
    iSyncFlag = OMX_FALSE;
    iBufferOverRun = OMX_FALSE;

    if (!IsAdded())
    {
        AddToScheduler();
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentMpeg4EncAO : constructed"));
}


//Active object destructor
OmxComponentMpeg4EncAO::~OmxComponentMpeg4EncAO()
{
    if (IsAdded())
    {
        RemoveFromScheduler();
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentMpeg4EncAO : destructed"));
}


OMX_ERRORTYPE OmxComponentMpeg4EncAO::SetConfig(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_INDEXTYPE nIndex,
    OMX_IN  OMX_PTR pComponentConfigStructure)
{
    OSCL_UNUSED_ARG(hComponent);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentMpeg4EncAO : SetConfig IN"));

    OMX_U32 PortIndex;

    OMX_ERRORTYPE ErrorType = OMX_ErrorNone;
    OMX_CONFIG_INTRAREFRESHVOPTYPE* pVideoIFrame;
    OMX_VIDEO_CONFIG_BITRATETYPE* pBitRateType;
    OMX_CONFIG_FRAMERATETYPE* pFrameRateType;



    if (NULL == pComponentConfigStructure)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentMpeg4EncAO : SetConfig error bad parameter"));
        return OMX_ErrorBadParameter;
    }

    switch (nIndex)
    {
        case OMX_IndexConfigVideoIntraVOPRefresh:
        {
            pVideoIFrame = (OMX_CONFIG_INTRAREFRESHVOPTYPE*) pComponentConfigStructure;
            PortIndex = pVideoIFrame->nPortIndex;

            if (PortIndex != iCompressedFormatPortNum)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentMpeg4EncAO : SetConfig error invalid port index"));
                return OMX_ErrorBadPortIndex;
            }

            /*Check Structure Header*/
            ErrorType = CheckHeader(pVideoIFrame, sizeof(OMX_CONFIG_INTRAREFRESHVOPTYPE));
            if (ErrorType != OMX_ErrorNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentMpeg4EncAO : SetConfig error param check failed"));
                return ErrorType;
            }

            //Call the RequestI frame routine of the encoder in case of setconfig call
            if (OMX_TRUE == pVideoIFrame->IntraRefreshVOP)
            {
                ipMpegEncoderObject->Mp4RequestIFrame();

            }
        }
        break;

        case OMX_IndexConfigVideoBitrate:
        {
            pBitRateType = (OMX_VIDEO_CONFIG_BITRATETYPE*) pComponentConfigStructure;
            PortIndex = pBitRateType->nPortIndex;

            if (PortIndex != iCompressedFormatPortNum)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentMpeg4EncAO : SetConfig error invalid port index"));
                return OMX_ErrorBadPortIndex;
            }

            /*Check Structure Header*/
            ErrorType = CheckHeader(pBitRateType, sizeof(OMX_VIDEO_CONFIG_BITRATETYPE));
            if (ErrorType != OMX_ErrorNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentMpeg4EncAO : SetConfig error param check failed"));
                return ErrorType;
            }

            //Call the corresponding routine of the encoder in case of setconfig call
            if (OMX_FALSE == (ipMpegEncoderObject->Mp4UpdateBitRate(pBitRateType->nEncodeBitrate)))
            {
                return OMX_ErrorUnsupportedSetting;
            }
            ipPorts[PortIndex]->VideoConfigBitRateType.nEncodeBitrate = pBitRateType->nEncodeBitrate;
        }
        break;

        case OMX_IndexConfigVideoFramerate:
        {
            pFrameRateType = (OMX_CONFIG_FRAMERATETYPE*) pComponentConfigStructure;
            PortIndex = pFrameRateType->nPortIndex;

            if (PortIndex != iCompressedFormatPortNum)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentMpeg4EncAO : SetConfig error invalid port index"));
                return OMX_ErrorBadPortIndex;
            }

            /*Check Structure Header*/
            ErrorType = CheckHeader(pFrameRateType, sizeof(OMX_CONFIG_FRAMERATETYPE));
            if (ErrorType != OMX_ErrorNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentMpeg4EncAO : SetConfig error param check failed"));
                return ErrorType;
            }

            //Call the corresponding routine of the encoder in case of setconfig call
            if (OMX_FALSE == (ipMpegEncoderObject->Mp4UpdateFrameRate(pFrameRateType->xEncodeFramerate)))
            {
                return OMX_ErrorUnsupportedSetting;
            }

            ipPorts[PortIndex]->VideoConfigFrameRateType.xEncodeFramerate = pFrameRateType->xEncodeFramerate;
        }
        break;

        default:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentMpeg4EncAO : SetConfig error Unsupported Index"));
            return OMX_ErrorUnsupportedIndex;
        }
        break;

    }


    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentMpeg4EncAO : SetConfig OUT"));

    return OMX_ErrorNone;

}



/** The Initialization function
 */
OMX_ERRORTYPE OmxComponentMpeg4EncAO::ComponentInit()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentMpeg4EncAO : ComponentInit IN"));

    OMX_ERRORTYPE Status = OMX_ErrorNone;

    if (OMX_TRUE == iIsInit)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentMpeg4EncAO : ComponentInit error incorrect operation"));
        return OMX_ErrorIncorrectStateOperation;
    }
    iIsInit = OMX_TRUE;


    if (!iCodecReady)
    {
        iCodecReady = OMX_TRUE;
    }

    //Check the H.263 parameters before initializing the encoder if there was any change in the SetParameter call
    if (MODE_H263 == iEncMode)
    {
        OMX_VIDEO_PARAM_H263TYPE* H263Param = (OMX_VIDEO_PARAM_H263TYPE*) & ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->VideoH263;

        if ((OMX_VIDEO_H263ProfileBaseline != H263Param->eProfile) ||
                (H263Param->eLevel > OMX_VIDEO_H263Level45) ||
                (OMX_TRUE == H263Param->bPLUSPTYPEAllowed) ||
                (0 == (H263Param->nAllowedPictureTypes & (OMX_VIDEO_PictureTypeI | OMX_VIDEO_PictureTypeP))) ||
                (0 != H263Param->nPictureHeaderRepetition))
        {

            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentMpeg4EncAO : ComponentInit Error, unsupported H.263 settings, OUT"));
            return OMX_ErrorUnsupportedSetting;
        }
    }


    //Library init routine
    Status = ipMpegEncoderObject->Mp4EncInit(iEncMode,
             ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.format.video,
             ipPorts[OMX_PORT_INPUTPORT_INDEX]->VideoOrientationType,
             ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.format.video,
             ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->VideoMpeg4,
             ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->VideoErrorCorrection,
             ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->VideoRateType,
             ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->VideoQuantType,
             ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->VideoMotionVector,
             ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->VideoIntraRefresh,
             ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->VideoH263,
             &(ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->ProfileLevel));

    iInputCurrLength = 0;

    //Used in dynamic port reconfiguration
    iFrameCount = 0;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentMpeg4EncAO : ComponentInit OUT"));

    return Status;

}

/** This function is called upon a transition to the idle or invalid state.
 *  Also it is called by the ComponentDestructor() function
 */
OMX_ERRORTYPE OmxComponentMpeg4EncAO::ComponentDeInit()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentMpeg4EncAO : ComponentDeInit IN"));

    OMX_ERRORTYPE Status = OMX_ErrorNone;

    iIsInit = OMX_FALSE;

    if (iCodecReady)
    {
        Status = ipMpegEncoderObject->Mp4EncDeinit();
        iCodecReady = OMX_FALSE;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentMpeg4EncAO : ComponentDeInit OUT"));

    return Status;

}

/* A component specific routine called from BufferMgmtWithoutMarker */
void OmxComponentMpeg4EncAO::ProcessInBufferFlag()
{
    iIsInputBufferEnded = OMX_FALSE;
}
