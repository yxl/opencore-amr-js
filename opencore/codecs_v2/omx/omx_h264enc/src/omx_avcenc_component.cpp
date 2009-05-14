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

#include "omx_avcenc_component.h"

#if PROXY_INTERFACE
#include "omx_proxy_interface.h"
#endif

const uint8 NAL_START_CODE[4] = {0, 0, 0, 1};

#define CONFIG_SIZE_AND_VERSION(param) \
	    param.nSize=sizeof(param); \
	    param.nVersion.s.nVersionMajor = SPECVERSIONMAJOR; \
	    param.nVersion.s.nVersionMinor = SPECVERSIONMINOR; \
	    param.nVersion.s.nRevision = SPECREVISION; \
	    param.nVersion.s.nStep = SPECSTEP;

// This function is called by OMX_GetHandle and it creates an instance of the avc component AO
OMX_ERRORTYPE AvcEncOmxComponentFactory(OMX_OUT OMX_HANDLETYPE* pHandle, OMX_IN OMX_PTR pAppData, OMX_PTR pProxy, OMX_STRING aOmxLibName, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount)
{
    OSCL_UNUSED_ARG(aOmxLibName);
    OSCL_UNUSED_ARG(aOmxLib);
    OSCL_UNUSED_ARG(aOsclUuid);
    OSCL_UNUSED_ARG(aRefCount);


    OmxComponentAvcEncAO* pOpenmaxAOType;
    OMX_ERRORTYPE Status;

    // move InitAvcOmxComponentFields content to actual constructor

    pOpenmaxAOType = (OmxComponentAvcEncAO*) OSCL_NEW(OmxComponentAvcEncAO, ());

    if (NULL == pOpenmaxAOType)
    {
        return OMX_ErrorInsufficientResources;
    }

    //Call the construct component to initialize OMX types
    Status = pOpenmaxAOType->ConstructComponent(pAppData, pProxy);

    *pHandle = pOpenmaxAOType->GetOmxHandle();

    return Status;
}


// This function is called by OMX_FreeHandle when component AO needs to be destroyed
OMX_ERRORTYPE AvcEncOmxComponentDestructor(OMX_IN OMX_HANDLETYPE pHandle, OMX_PTR &aOmxLib, OMX_PTR aOsclUuid, OMX_U32 &aRefCount)
{
    OSCL_UNUSED_ARG(aOmxLib);
    OSCL_UNUSED_ARG(aOsclUuid);
    OSCL_UNUSED_ARG(aRefCount);

    // get pointer to component AO
    OmxComponentAvcEncAO* pOpenmaxAOType = (OmxComponentAvcEncAO*)((OMX_COMPONENTTYPE*)pHandle)->pComponentPrivate;

    // clean up decoder, OMX component stuff
    pOpenmaxAOType->DestroyComponent();

    // destroy the AO class
    OSCL_DELETE(pOpenmaxAOType);

    return OMX_ErrorNone;
}

#if DYNAMIC_LOAD_OMX_AVCENC_COMPONENT

class AvcEncOmxSharedLibraryInterface: public OsclSharedLibraryInterface,
            public OmxSharedLibraryInterface

{
    public:
        static AvcEncOmxSharedLibraryInterface *Instance()
        {
            static AvcEncOmxSharedLibraryInterface omxinterface;
            return &omxinterface;
        };

        OsclAny *QueryOmxComponentInterface(const OsclUuid& aOmxTypeId, const OsclUuid& aInterfaceId)
        {
            if (PV_OMX_AVCENC_UUID == aOmxTypeId)
            {
                if (PV_OMX_CREATE_INTERFACE == aInterfaceId)
                {
                    return ((OsclAny*)(&AvcEncOmxComponentFactory));
                }
                else if (PV_OMX_DESTROY_INTERFACE == aInterfaceId)
                {
                    return ((OsclAny*)(&AvcEncOmxComponentDestructor));
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
        AvcEncOmxSharedLibraryInterface() {};
};

// function to obtain the interface object from the shared library
extern "C"
{
    OSCL_EXPORT_REF OsclAny* PVGetInterface()
    {
        return AvcEncOmxSharedLibraryInterface::Instance();
    }
}

#endif


OMX_ERRORTYPE OmxComponentAvcEncAO::ConstructComponent(OMX_PTR pAppData, OMX_PTR pProxy)
{
    ComponentPortType *pInPort, *pOutPort;
    OMX_ERRORTYPE Status;

    iNumPorts = 2;
    iCompressedFormatPortNum = OMX_PORT_OUTPUTPORT_INDEX;
    iOmxComponent.nSize = sizeof(OMX_COMPONENTTYPE);
    iOmxComponent.pComponentPrivate = (OMX_PTR) this;  // pComponentPrivate points to THIS component AO class
    ipComponentProxy = pProxy;
    iOmxComponent.pApplicationPrivate = pAppData; // init the App data

    oscl_memset((void *)iNALSizeArray, 0, MAX_NAL_PER_FRAME * sizeof(int32));
    iNALCount = 0;
    iNALSizeSum = 0;
    iEndOfOutputFrame = OMX_FALSE;

#if PROXY_INTERFACE
    iPVCapabilityFlags.iIsOMXComponentMultiThreaded = OMX_TRUE;

    iOmxComponent.SendCommand = OmxComponentAvcEncAO::BaseComponentProxySendCommand;
    iOmxComponent.GetParameter = OmxComponentAvcEncAO::BaseComponentProxyGetParameter;
    iOmxComponent.SetParameter = OmxComponentAvcEncAO::BaseComponentProxySetParameter;
    iOmxComponent.GetConfig = OmxComponentAvcEncAO::BaseComponentProxyGetConfig;
    iOmxComponent.SetConfig = OmxComponentAvcEncAO::BaseComponentProxySetConfig;
    iOmxComponent.GetExtensionIndex = OmxComponentAvcEncAO::BaseComponentProxyGetExtensionIndex;
    iOmxComponent.GetState = OmxComponentAvcEncAO::BaseComponentProxyGetState;
    iOmxComponent.UseBuffer = OmxComponentAvcEncAO::BaseComponentProxyUseBuffer;
    iOmxComponent.AllocateBuffer = OmxComponentAvcEncAO::BaseComponentProxyAllocateBuffer;
    iOmxComponent.FreeBuffer = OmxComponentAvcEncAO::BaseComponentProxyFreeBuffer;
    iOmxComponent.EmptyThisBuffer = OmxComponentAvcEncAO::BaseComponentProxyEmptyThisBuffer;
    iOmxComponent.FillThisBuffer = OmxComponentAvcEncAO::BaseComponentProxyFillThisBuffer;

#else
    iPVCapabilityFlags.iIsOMXComponentMultiThreaded = OMX_FALSE;

    iOmxComponent.SendCommand = OmxComponentAvcEncAO::BaseComponentSendCommand;
    iOmxComponent.GetParameter = OmxComponentAvcEncAO::BaseComponentGetParameter;
    iOmxComponent.SetParameter = OmxComponentAvcEncAO::BaseComponentSetParameter;
    iOmxComponent.GetConfig = OmxComponentAvcEncAO::BaseComponentGetConfig;
    iOmxComponent.SetConfig = OmxComponentAvcEncAO::BaseComponentSetConfig;
    iOmxComponent.GetExtensionIndex = OmxComponentAvcEncAO::BaseComponentGetExtensionIndex;
    iOmxComponent.GetState = OmxComponentAvcEncAO::BaseComponentGetState;
    iOmxComponent.UseBuffer = OmxComponentAvcEncAO::BaseComponentUseBuffer;
    iOmxComponent.AllocateBuffer = OmxComponentAvcEncAO::BaseComponentAllocateBuffer;
    iOmxComponent.FreeBuffer = OmxComponentAvcEncAO::BaseComponentFreeBuffer;
    iOmxComponent.EmptyThisBuffer = OmxComponentAvcEncAO::BaseComponentEmptyThisBuffer;
    iOmxComponent.FillThisBuffer = OmxComponentAvcEncAO::BaseComponentFillThisBuffer;
#endif

    iOmxComponent.SetCallbacks = OmxComponentAvcEncAO::BaseComponentSetCallbacks;
    iOmxComponent.nVersion.s.nVersionMajor = SPECVERSIONMAJOR;
    iOmxComponent.nVersion.s.nVersionMinor = SPECVERSIONMINOR;
    iOmxComponent.nVersion.s.nRevision = SPECREVISION;
    iOmxComponent.nVersion.s.nStep = SPECSTEP;

    // PV capability
#if defined(TEST_FULL_AVC_FRAME_MODE)
    /* output buffers based on frame boundaries instead of NAL boundaries and specify NAL boundaries through
     * through OMX_EXTRADATA structures appended on the end of the buffer
     */
    iPVCapabilityFlags.iOMXComponentSupportsExternalInputBufferAlloc = OMX_TRUE;
    iPVCapabilityFlags.iOMXComponentSupportsExternalOutputBufferAlloc = OMX_TRUE;
    iPVCapabilityFlags.iOMXComponentSupportsMovableInputBuffers = OMX_TRUE;
    iPVCapabilityFlags.iOMXComponentSupportsPartialFrames = OMX_TRUE;
    iPVCapabilityFlags.iOMXComponentUsesNALStartCodes = OMX_FALSE;
    iPVCapabilityFlags.iOMXComponentCanHandleIncompleteFrames = OMX_TRUE;
    iPVCapabilityFlags.iOMXComponentUsesFullAVCFrames = OMX_TRUE;
#elif defined(TEST_FULL_AVC_FRAME_MODE_SC)
    /* output buffers based on frame boundaries instead of NAL boundaries and specify NAL boundaries
     * with NAL start codes
     */
    iPVCapabilityFlags.iOMXComponentSupportsExternalInputBufferAlloc = OMX_TRUE;
    iPVCapabilityFlags.iOMXComponentSupportsExternalOutputBufferAlloc = OMX_TRUE;
    iPVCapabilityFlags.iOMXComponentSupportsMovableInputBuffers = OMX_TRUE;
    iPVCapabilityFlags.iOMXComponentSupportsPartialFrames = OMX_TRUE;
    iPVCapabilityFlags.iOMXComponentUsesNALStartCodes = OMX_TRUE;
    iPVCapabilityFlags.iOMXComponentCanHandleIncompleteFrames = OMX_TRUE;
    iPVCapabilityFlags.iOMXComponentUsesFullAVCFrames = OMX_TRUE;
#else
    iPVCapabilityFlags.iOMXComponentSupportsExternalInputBufferAlloc = OMX_TRUE;
    iPVCapabilityFlags.iOMXComponentSupportsExternalOutputBufferAlloc = OMX_TRUE;
    iPVCapabilityFlags.iOMXComponentSupportsMovableInputBuffers = OMX_TRUE;
    iPVCapabilityFlags.iOMXComponentSupportsPartialFrames = OMX_TRUE;
    iPVCapabilityFlags.iOMXComponentUsesNALStartCodes = OMX_FALSE;
    iPVCapabilityFlags.iOMXComponentCanHandleIncompleteFrames = OMX_TRUE;
    iPVCapabilityFlags.iOMXComponentUsesFullAVCFrames = OMX_FALSE;
#endif

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
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.nBufferCountActual = NUMBER_INPUT_BUFFER_AVCENC;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.nBufferCountMin = 1;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.nBufferSize = INPUT_BUFFER_SIZE_AVCENC;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.bEnabled = OMX_TRUE;
    ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.bPopulated = OMX_FALSE;


    /** Domain specific section for output avc port */
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.nPortIndex = OMX_PORT_OUTPUTPORT_INDEX;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.eDomain = OMX_PortDomainVideo;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.format.video.cMIMEType = (OMX_STRING)"video/avc";
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.format.video.eCompressionFormat = OMX_VIDEO_CodingAVC;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.format.video.pNativeRender = 0;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.format.video.bFlagErrorConcealment = OMX_FALSE;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.format.video.eColorFormat = OMX_COLOR_FormatUnused;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.format.video.nFrameWidth = 176;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.format.video.nFrameHeight = 144;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.format.video.nBitrate = 64000;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.format.video.xFramerate = (15 << 16);
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.eDir = OMX_DirOutput;
    //Set to a default value, will change later during setparameter call
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.nBufferCountActual = NUMBER_OUTPUT_BUFFER_AVCENC;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.nBufferCountMin = 1;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.nBufferSize = OUTPUT_BUFFER_SIZE_AVCENC;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.bEnabled = OMX_TRUE;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.bPopulated = OMX_FALSE;


    //OMX_VIDEO_PARAM_AVCTYPE	//Default values for avc video output param port
    oscl_memset(&ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->VideoAvc, 0, sizeof(OMX_VIDEO_PARAM_AVCTYPE));
    SetHeader(&ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->VideoAvc, sizeof(OMX_VIDEO_PARAM_AVCTYPE));

    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->VideoAvc.nPortIndex = OMX_PORT_OUTPUTPORT_INDEX;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->VideoAvc.eProfile = OMX_VIDEO_AVCProfileBaseline;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->VideoAvc.eLevel = OMX_VIDEO_AVCLevel1b;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->VideoAvc.nPFrames = 0xFFFFFFFF;	//Default value
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->VideoAvc.nBFrames = 0;		//No support for B frames
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->VideoAvc.nAllowedPictureTypes = OMX_VIDEO_PictureTypeI | OMX_VIDEO_PictureTypeP;

    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->VideoAvc.nRefFrames = 1;	//Only support this value
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->VideoAvc.eLoopFilterMode = OMX_VIDEO_AVCLoopFilterEnable;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->VideoAvc.bEnableFMO = OMX_FALSE;	//Default value is false

    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->VideoAvc.bFrameMBsOnly = OMX_TRUE;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->VideoAvc.bMBAFF = OMX_FALSE;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->VideoAvc.bEntropyCodingCABAC = OMX_FALSE;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->VideoAvc.bWeightedPPrediction = OMX_FALSE;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->VideoAvc.bDirect8x8Inference = OMX_FALSE;


    //OMX_VIDEO_PARAM_PROFILELEVELTYPE structure
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->ProfileLevel.nPortIndex = OMX_PORT_OUTPUTPORT_INDEX;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->ProfileLevel.nProfileIndex = 0;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->ProfileLevel.eProfile = OMX_VIDEO_AVCProfileBaseline;
    ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->ProfileLevel.eLevel = OMX_VIDEO_AVCLevel1b;


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
    pOutPort->VideoParam[0].eCompressionFormat = OMX_VIDEO_CodingAVC;
    pOutPort->VideoParam[0].xFramerate = (15 << 16);


    //OMX_CONFIG_ROTATIONTYPE SETTINGS ON INPUT PORT
    SetHeader(&pInPort->VideoOrientationType, sizeof(OMX_CONFIG_ROTATIONTYPE));
    pInPort->VideoOrientationType.nPortIndex = OMX_PORT_INPUTPORT_INDEX;
    pInPort->VideoOrientationType.nRotation = -1;  //For all the YUV formats that are other than RGB


    //OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE settings of output port
    oscl_memset(&pOutPort->VideoErrorCorrection, 0, sizeof(OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE));
    SetHeader(&pOutPort->VideoErrorCorrection, sizeof(OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE));
    pOutPort->VideoErrorCorrection.nPortIndex = OMX_PORT_OUTPUTPORT_INDEX;
    pOutPort->VideoErrorCorrection.bEnableDataPartitioning = OMX_FALSE;


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
    pOutPort->VideoQuantType.nQpI = 0;	//Not required by encoder
    pOutPort->VideoQuantType.nQpP = 0;	//Default is 0
    pOutPort->VideoQuantType.nQpB = 0;	//Not required by encoder


    //OMX_VIDEO_PARAM_VBSMCTYPE settings of output port
    oscl_memset(&pOutPort->VideoBlockMotionSize, 0, sizeof(OMX_VIDEO_PARAM_VBSMCTYPE));
    SetHeader(&pOutPort->VideoBlockMotionSize, sizeof(OMX_VIDEO_PARAM_VBSMCTYPE));
    pOutPort->VideoBlockMotionSize.nPortIndex = OMX_PORT_OUTPUTPORT_INDEX;
    pOutPort->VideoBlockMotionSize.b16x16 = OMX_TRUE;		//Encoder only support this mode


    //OMX_VIDEO_PARAM_MOTIONVECTORTYPE settings of output port
    oscl_memset(&pOutPort->VideoMotionVector, 0, sizeof(OMX_VIDEO_PARAM_MOTIONVECTORTYPE));
    SetHeader(&pOutPort->VideoMotionVector, sizeof(OMX_VIDEO_PARAM_MOTIONVECTORTYPE));
    pOutPort->VideoMotionVector.nPortIndex = OMX_PORT_OUTPUTPORT_INDEX;
    pOutPort->VideoMotionVector.eAccuracy = OMX_Video_MotionVectorQuarterPel;
    pOutPort->VideoMotionVector.bUnrestrictedMVs = OMX_TRUE;		//Only support true
    pOutPort->VideoMotionVector.sXSearchRange = 16;
    pOutPort->VideoMotionVector.sYSearchRange = 16;


    //OMX_VIDEO_PARAM_INTRAREFRESHTYPE settings of output port
    oscl_memset(&pOutPort->VideoIntraRefresh, 0, sizeof(OMX_VIDEO_PARAM_INTRAREFRESHTYPE));
    SetHeader(&pOutPort->VideoIntraRefresh, sizeof(OMX_VIDEO_PARAM_INTRAREFRESHTYPE));
    pOutPort->VideoIntraRefresh.nPortIndex = OMX_PORT_OUTPUTPORT_INDEX;
    pOutPort->VideoIntraRefresh.eRefreshMode = OMX_VIDEO_IntraRefreshCyclic;
    pOutPort->VideoIntraRefresh.nCirMBs = 0;

    //OMX_VIDEO_PARAM_AVCSLICEFMO settings on output port
    oscl_memset(&pOutPort->AvcSliceFMO, 0, sizeof(OMX_VIDEO_PARAM_AVCSLICEFMO));
    SetHeader(&pOutPort->AvcSliceFMO, sizeof(OMX_VIDEO_PARAM_AVCSLICEFMO));
    pOutPort->AvcSliceFMO.nPortIndex = OMX_PORT_OUTPUTPORT_INDEX;
    pOutPort->AvcSliceFMO.eSliceMode = OMX_VIDEO_SLICEMODE_AVCDefault;
    pOutPort->AvcSliceFMO.nNumSliceGroups = 1;
    pOutPort->AvcSliceFMO.nSliceGroupMapType = 1;			//Only support map type of 1


    //OMX_CONFIG_INTRAREFRESHVOPTYPE settings of output port
    oscl_memset(&pOutPort->VideoIFrame, 0, sizeof(OMX_CONFIG_INTRAREFRESHVOPTYPE));
    SetHeader(&pOutPort->VideoIFrame, sizeof(OMX_CONFIG_INTRAREFRESHVOPTYPE));
    pOutPort->VideoIFrame.nPortIndex = OMX_PORT_OUTPUTPORT_INDEX;
    pOutPort->VideoIFrame.IntraRefreshVOP = OMX_FALSE;


    //Construct the encoder object
    if (ipAvcEncoderObject)
    {
        OSCL_DELETE(ipAvcEncoderObject);
        ipAvcEncoderObject = NULL;
    }

    ipAvcEncoderObject = OSCL_NEW(AvcEncoder_OMX, ());

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

OMX_ERRORTYPE OmxComponentAvcEncAO::DestroyComponent()
{
    if (OMX_FALSE != iIsInit)
    {
        ComponentDeInit();
    }

    //Destroy the base class now
    DestroyBaseComponent();

    if (ipAvcEncoderObject)
    {
        OSCL_DELETE(ipAvcEncoderObject);
        ipAvcEncoderObject = NULL;
    }

    if (ipAppPriv)
    {
        ipAppPriv->CompHandle = NULL;

        oscl_free(ipAppPriv);
        ipAppPriv = NULL;
    }

    return OMX_ErrorNone;
}


void OmxComponentAvcEncAO::ProcessData()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAvcEncAO : ProcessData IN"));

    QueueType* pInputQueue = ipPorts[OMX_PORT_INPUTPORT_INDEX]->pBufferQueue;
    QueueType* pOutputQueue = ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->pBufferQueue;

    ComponentPortType*	pInPort = ipPorts[OMX_PORT_INPUTPORT_INDEX];

    OMX_U8*					pOutBuffer;
    OMX_U32					OutputLength;
    AVCEnc_Status			EncodeReturn = AVCENC_SUCCESS;
    OMX_COMPONENTTYPE*		pHandle = &iOmxComponent;

    if ((!iIsInputBufferEnded) || (iEndofStream))
    {
        //Check whether prev output bufer has been released or not
        if (OMX_TRUE == iNewOutBufRequired)
        {
            //Check whether a new output buffer is available or not

            if (0 == (GetQueueNumElem(pOutputQueue)))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAvcEncAO : ProcessData OUT output buffer unavailable"));
                return;
            }

            ipOutputBuffer = (OMX_BUFFERHEADERTYPE*) DeQueue(pOutputQueue);

            OSCL_ASSERT(NULL != ipOutputBuffer);
            if (ipOutputBuffer == NULL)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentAvcEncAO : ProcessData ERR OUT output buffer cannot be dequeued"));
                return;
            }
            ipOutputBuffer->nFilledLen = 0;
            iNewOutBufRequired = OMX_FALSE;

            oscl_memset((void *)iNALSizeArray, 0, iNALCount * sizeof(int32));
            iNALCount = 0;
            iNALSizeSum = 0;

            if (iPVCapabilityFlags.iOMXComponentUsesNALStartCodes)
            {
                oscl_memcpy(ipOutputBuffer->pBuffer + ipOutputBuffer->nOffset + ipOutputBuffer->nFilledLen, &NAL_START_CODE, sizeof(uint8) * 4);
                ipOutputBuffer->nFilledLen += 4;
                iNALSizeSum += 4;
            }


            /* If some output data was left to be send from the last processing due to
             * unavailability of required number of output buffers,
             * copy it now and send back before processing new input frame */
            if (iInternalOutBufFilledLen > 0)
            {
                if (OMX_FALSE == CopyDataToOutputBuffer())
                {
                    //We fell short of output buffers, exit now and wait for some more buffers to get queued
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAvcEncAO : ProcessData OUT output buffer unavailable"));
                    return;
                }
                else
                {
                    ManageFrameBoundaries();

                    //Dequeue new output buffer to continue encoding the next frame
                    if (0 == (GetQueueNumElem(pOutputQueue)))
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAvcEncAO : ProcessData OUT output buffer unavailable"));
                        return;
                    }
                    ipOutputBuffer = (OMX_BUFFERHEADERTYPE*) DeQueue(pOutputQueue);

                    OSCL_ASSERT(NULL != ipOutputBuffer);
                    if (ipOutputBuffer == NULL)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentAvcEncAO : ProcessData OUT ERR output buffer cannot be dequeued"));
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
            OMX_S32 filledLength = ipOutputBuffer->nOffset + ipOutputBuffer->nFilledLen;
            pOutBuffer = ipOutputBuffer->pBuffer + (OMX_U32)filledLength;

            if (iPVCapabilityFlags.iOMXComponentUsesFullAVCFrames && !iPVCapabilityFlags.iOMXComponentUsesNALStartCodes)
            {
                OutputLength = (OMX_U32)(((OMX_S32)ipOutputBuffer->nAllocLen - filledLength - (46 + 4 * (iNALCount + 1))) > 0) ? (ipOutputBuffer->nAllocLen - filledLength - (46 + 4 * (iNALCount + 1))) : 0;
                // (20 + 4 * (iNALCount + 1) + 20 + 6) is size of extra data
            }
            else
            {
                OutputLength = (OMX_U32)(((OMX_S32)ipOutputBuffer->nAllocLen - filledLength) > 0) ? (ipOutputBuffer->nAllocLen - filledLength) : 0;
            }

            //Output buffer is passed as a short pointer
            EncodeReturn = ipAvcEncoderObject->AvcEncodeVideo(pOutBuffer,
                           &OutputLength,
                           &iBufferOverRun,
                           &ipInternalOutBuffer,
                           ipFrameDecodeBuffer,
                           &iInputCurrLength,
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
                    ipOutputBuffer->nFilledLen += OutputLength;
                }
                else
                {
                    iInternalOutBufFilledLen = OutputLength;
                    iBufferOverRun = OMX_FALSE;
                    CopyDataToOutputBuffer();

                }	//else loop of if (OMX_FALSE == iMantainOutInternalBuffer)
            }	//if (OutputLength > 0)	 loop

            //If encoder returned error in case of frame skip/corrupt frame, report it to the client via a callback
            if (((AVCENC_SKIPPED_PICTURE == EncodeReturn) || (AVCENC_FAIL == EncodeReturn))
                    && (OMX_FALSE == iEndofStream))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAvcEncAO : Frame skipped, ProcessData ErrorStreamCorrupt callback send"));

                (*(ipCallbacks->EventHandler))
                (pHandle,
                 iCallbackData,
                 OMX_EventError,
                 OMX_ErrorStreamCorrupt,
                 0,
                 NULL);
            }

            //Return the input buffer that has been consumed fully
            if ((AVCENC_PICTURE_READY == EncodeReturn) ||
                    (AVCENC_SKIPPED_PICTURE == EncodeReturn) ||
                    (AVCENC_FAIL == EncodeReturn))
            {
                ipInputBuffer->nFilledLen = 0;
                ReturnInputBuffer(ipInputBuffer, pInPort);
                ipInputBuffer = NULL;

                iIsInputBufferEnded = OMX_TRUE;
                iInputCurrLength = 0;

                iFrameCount++;
            }

            if (AVCENC_PICTURE_READY == EncodeReturn)
            {
                iEndOfOutputFrame = OMX_TRUE;
            }
        }


        /* If EOS flag has come from the client & there are no more
         * input buffers to decode, send the callback to the client
         */
        if (OMX_TRUE == iEndofStream)
        {
            if (((0 == iInputCurrLength) || (AVCENC_FAIL == EncodeReturn)) &&
                    (0 == iInternalOutBufFilledLen))
            {

                (*(ipCallbacks->EventHandler))
                (pHandle,
                 iCallbackData,
                 OMX_EventBufferFlag,
                 1,
                 OMX_BUFFERFLAG_EOS,
                 NULL);
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAvcEncAO : ProcessData EOS callback sent"));

                ManageFrameBoundaries();

                //Mark this flag false once the callback has been send back
                iEndofStream = OMX_FALSE;

                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAvcEncAO : ProcessData OUT"));
                return;
            }

        }

        if (!iPVCapabilityFlags.iOMXComponentUsesNALStartCodes)
        {
            if (iEndOfOutputFrame || ((ipOutputBuffer->nFilledLen > 0) && (OMX_FALSE == iNewOutBufRequired)))
            {
                ManageFrameBoundaries();
            }
        }
        else if (ipOutputBuffer->nFilledLen > 4) // therefore only if more than just start code in buffer
        {
            if (iEndOfOutputFrame || (OMX_FALSE == iNewOutBufRequired))
            {
                ManageFrameBoundaries();
            }
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

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAvcEncAO : ProcessData OUT"));
}


OMX_BOOL OmxComponentAvcEncAO::CopyDataToOutputBuffer()
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
            //Mark the sync Flag in every piece of IDR NAL
            if (OMX_TRUE == iSyncFlag)
            {
                ipOutputBuffer->nFlags |= OMX_BUFFERFLAG_SYNCFRAME;
            }
            //Return the partial output buffer and try to fetch a new output buffer for filling the remaining data
            ReturnOutputBuffer(ipOutputBuffer, pOutPort);

            //Check whether a new output buffer is available or not
            if (0 == (GetQueueNumElem(pOutputQueue)))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAvcEncAO : CopyDatatoOutputBuffer OUT output buffer unavailable"));
                return OMX_FALSE;
            }

            ipOutputBuffer = (OMX_BUFFERHEADERTYPE*) DeQueue(pOutputQueue);

            OSCL_ASSERT(NULL != ipOutputBuffer);
            if (ipOutputBuffer == NULL)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentAvcEncAO : CopyDatatoOutputBuffer ERR OUT output buffer cannot be dequeued"));
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

void OmxComponentAvcEncAO::ComponentGetRolesOfComponent(OMX_STRING* aRoleString)
{
    *aRoleString = (OMX_STRING)"video_encoder.avc";
}


//Component constructor
OmxComponentAvcEncAO::OmxComponentAvcEncAO()
{
    ipAvcEncoderObject = NULL;
    ipInternalOutBuffer = NULL;
    iInternalOutBufFilledLen = 0;
    iSyncFlag = OMX_FALSE;
    iBufferOverRun = OMX_FALSE;

    if (!IsAdded())
    {
        AddToScheduler();
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAvcEncAO : constructed"));
}


//Active object destructor
OmxComponentAvcEncAO::~OmxComponentAvcEncAO()
{
    if (IsAdded())
    {
        RemoveFromScheduler();
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAvcEncAO : destructed"));
}


OMX_ERRORTYPE OmxComponentAvcEncAO::SetConfig(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_INDEXTYPE nIndex,
    OMX_IN  OMX_PTR pComponentConfigStructure)
{
    OSCL_UNUSED_ARG(hComponent);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAvcEncAO : SetConfig IN"));

    OMX_U32 PortIndex;

    OMX_ERRORTYPE ErrorType = OMX_ErrorNone;
    OMX_CONFIG_INTRAREFRESHVOPTYPE* pAvcIFrame;
    OMX_VIDEO_CONFIG_BITRATETYPE* pBitRateType;
    OMX_CONFIG_FRAMERATETYPE* pFrameRateType;



    if (NULL == pComponentConfigStructure)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAvcEncAO : SetConfig error bad parameter"));
        return OMX_ErrorBadParameter;
    }

    switch (nIndex)
    {
        case OMX_IndexConfigVideoIntraVOPRefresh:
        {
            pAvcIFrame = (OMX_CONFIG_INTRAREFRESHVOPTYPE*) pComponentConfigStructure;
            PortIndex = pAvcIFrame->nPortIndex;

            if (PortIndex != iCompressedFormatPortNum)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAvcEncAO : SetConfig error invalid port index"));
                return OMX_ErrorBadPortIndex;
            }

            /*Check Structure Header*/
            ErrorType = CheckHeader(pAvcIFrame, sizeof(OMX_CONFIG_INTRAREFRESHVOPTYPE));
            if (ErrorType != OMX_ErrorNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAvcEncAO : SetConfig error param check failed"));
                return ErrorType;
            }

            //Call the RequestI frame routine of the encoder in case of setconfig call
            if (OMX_TRUE == pAvcIFrame->IntraRefreshVOP)
            {
                ipAvcEncoderObject->AvcRequestIFrame();

            }
        }
        break;

        case OMX_IndexConfigVideoBitrate:
        {
            pBitRateType = (OMX_VIDEO_CONFIG_BITRATETYPE*) pComponentConfigStructure;
            PortIndex = pBitRateType->nPortIndex;

            if (PortIndex != iCompressedFormatPortNum)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAvcEncAO : SetConfig error invalid port index"));
                return OMX_ErrorBadPortIndex;
            }

            /*Check Structure Header*/
            ErrorType = CheckHeader(pBitRateType, sizeof(OMX_VIDEO_CONFIG_BITRATETYPE));
            if (ErrorType != OMX_ErrorNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAvcEncAO : SetConfig error param check failed"));
                return ErrorType;
            }

            //Call the corresponding routine of the encoder in case of setconfig call
            if (OMX_FALSE == (ipAvcEncoderObject->AvcUpdateBitRate(pBitRateType->nEncodeBitrate)))
            {
                return OMX_ErrorBadParameter;

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
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAvcEncAO : SetConfig error invalid port index"));
                return OMX_ErrorBadPortIndex;
            }

            /*Check Structure Header*/
            ErrorType = CheckHeader(pFrameRateType, sizeof(OMX_CONFIG_FRAMERATETYPE));
            if (ErrorType != OMX_ErrorNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAvcEncAO : SetConfig error param check failed"));
                return ErrorType;
            }

            //Call the corresponding routine of the encoder in case of setconfig call
            if (OMX_FALSE == (ipAvcEncoderObject->AvcUpdateFrameRate(pFrameRateType->xEncodeFramerate)))
            {
                return OMX_ErrorBadParameter;
            }

            ipPorts[PortIndex]->VideoConfigFrameRateType.xEncodeFramerate = pFrameRateType->xEncodeFramerate;
        }
        break;

        default:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAvcEncAO : SetConfig error Unsupported Index"));
            return OMX_ErrorUnsupportedIndex;
        }
        break;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAvcEncAO : SetConfig OUT"));
    return OMX_ErrorNone;

}



/** The Initialization function
 */
OMX_ERRORTYPE OmxComponentAvcEncAO::ComponentInit()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAvcEncAO : ComponentInit IN"));

    OMX_ERRORTYPE Status = OMX_ErrorNone;

    if (OMX_TRUE == iIsInit)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAvcEncAO : ComponentInit error incorrect operation"));
        return OMX_ErrorIncorrectStateOperation;
    }
    iIsInit = OMX_TRUE;


    if (!iCodecReady)
    {
        iCodecReady = OMX_TRUE;
    }

    //Verify the parameters and return failure in case they cannot be supported by our encoder
    OMX_VIDEO_PARAM_AVCTYPE* pAvcParam = (OMX_VIDEO_PARAM_AVCTYPE*) & ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->VideoAvc;
    OMX_VIDEO_PARAM_AVCSLICEFMO* pAvcSliceFMO = (OMX_VIDEO_PARAM_AVCSLICEFMO*) & ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->AvcSliceFMO;

    //FMO enabled is not supported in the encoder
    if ((pAvcParam->nBFrames > 0) ||
            (0 == (pAvcParam->nAllowedPictureTypes &
                   (OMX_VIDEO_PictureTypeI | OMX_VIDEO_PictureTypeP))) ||
            (OMX_FALSE == pAvcParam->bFrameMBsOnly) ||
            (OMX_TRUE == pAvcParam->bMBAFF) ||
            (OMX_TRUE == pAvcParam->bEntropyCodingCABAC) ||
            (OMX_TRUE == pAvcParam->bWeightedPPrediction) ||
            (OMX_TRUE == pAvcParam->bDirect8x8Inference) ||
            (OMX_TRUE == pAvcParam->bEnableFMO) ||
            (1 != pAvcSliceFMO->nSliceGroupMapType))
    {
        return OMX_ErrorUnsupportedSetting;
    }


    //Library init routine
    Status = ipAvcEncoderObject->AvcEncInit(
                 ipPorts[OMX_PORT_INPUTPORT_INDEX]->PortParam.format.video,
                 ipPorts[OMX_PORT_INPUTPORT_INDEX]->VideoOrientationType,
                 ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->PortParam.format.video,
                 ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->VideoAvc,
                 ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->VideoRateType,
                 ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->VideoQuantType,
                 ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->VideoMotionVector,
                 ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->VideoIntraRefresh,
                 ipPorts[OMX_PORT_OUTPUTPORT_INDEX]->VideoBlockMotionSize);

    iInputCurrLength = 0;

    //Used in dynamic port reconfiguration
    iFrameCount = 0;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAvcEncAO : ComponentInit OUT"));

    return Status;

}

/** This function is called upon a transition to the idle or invalid state.
 *  Also it is called by the ComponentDestructor() function
 */
OMX_ERRORTYPE OmxComponentAvcEncAO::ComponentDeInit()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAvcEncAO : ComponentDeInit IN"));

    OMX_ERRORTYPE Status = OMX_ErrorNone;

    iIsInit = OMX_FALSE;

    if (iCodecReady)
    {
        Status = ipAvcEncoderObject->AvcEncDeinit();
        iCodecReady = OMX_FALSE;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "OmxComponentAvcEncAO : ComponentDeInit OUT"));

    return Status;

}

/* A component specific routine called from BufferMgmtWithoutMarker */
void OmxComponentAvcEncAO::ProcessInBufferFlag()
{
    iIsInputBufferEnded = OMX_FALSE;
}

/////////////////////////////////////////////////////////////////////////////
OMX_BOOL OmxComponentAvcEncAO::AppendExtraDataToBuffer(OMX_BUFFERHEADERTYPE* aOutputBuffer,
        OMX_EXTRADATATYPE aType,
        OMX_U8* aExtraData,
        OMX_U8 aDataLength)

{
    // This function is used to append AVC NAL info to the buffer using the OMX_EXTRADATA_TYPE structure, when
    // a component requires buffers with full AVC frames rather than just NALs
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_NOTICE,
                    (0, "OmxComponentAvcEncAO::AppendExtraDataToBuffer() In"));


    if ((aType != OMX_ExtraDataNone) && (aExtraData != NULL) && (aOutputBuffer->pBuffer != NULL))
    {
        const uint32 sizeOfExtraDataStruct = 20; // 20 is the number of bytes for the OMX_OTHER_EXTRADATATYPE structure (minus the data hint member)

        OMX_OTHER_EXTRADATATYPE extra;
        OMX_OTHER_EXTRADATATYPE terminator;

        CONFIG_SIZE_AND_VERSION(extra);
        CONFIG_SIZE_AND_VERSION(terminator);

        extra.nPortIndex = OMX_PORT_OUTPUTPORT_INDEX;
        terminator.nPortIndex = OMX_PORT_OUTPUTPORT_INDEX;

        extra.eType = aType;
        extra.nSize = (sizeOfExtraDataStruct + aDataLength + 3) & ~3; // size + padding for byte alignment
        extra.nDataSize = aDataLength;

        // fill in fields for terminator
        terminator.eType = OMX_ExtraDataNone;
        terminator.nDataSize = 0;

        // make sure there is enough room in the buffer
        if (aOutputBuffer->nAllocLen < (aOutputBuffer->nOffset + aOutputBuffer->nFilledLen + sizeOfExtraDataStruct + aDataLength + terminator.nSize + 6))
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "OmxComponentAvcEncAO::AppendExtraDataToBuffer()  - Error (not enough room in buffer) appending extra data to Buffer 0x%x, TS=%d", aOutputBuffer->pBuffer, iOutputTimeStamp));

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "OmxComponentAvcEncAO::AppendExtraDataToBuffer() Out"));

            return OMX_FALSE;
        }

        // copy extra data into buffer
        // need to align to 4 bytes
        OMX_U8* buffer = aOutputBuffer->pBuffer + aOutputBuffer->nOffset + aOutputBuffer->nFilledLen;
        buffer = (OMX_U8*)(((OMX_U32) buffer + 3) & ~3);

        oscl_memcpy(buffer, &extra, sizeOfExtraDataStruct);
        oscl_memcpy(buffer + sizeOfExtraDataStruct, aExtraData, aDataLength);
        buffer += extra.nSize;

        oscl_memcpy(buffer, &terminator, terminator.nSize);

        // flag buffer
        aOutputBuffer->nFlags |= OMX_BUFFERFLAG_EXTRADATA;

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "OmxComponentAvcEncAO::AppendExtraDataToBuffer()  - Appending extra data to Buffer 0x%x, TS=%d", aOutputBuffer->pBuffer, iOutputTimeStamp));

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "OmxComponentAvcEncAO::AppendExtraDataToBuffer() Out"));

        return OMX_TRUE;
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "OmxComponentAvcEncAO::AppendExtraDataToBuffer() Out"));

        return OMX_FALSE;
    }
}

void OmxComponentAvcEncAO::ManageFrameBoundaries()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_NOTICE,
                    (0, "OmxComponentAvcEncAO::ManageFrameBoundaries() In"));

    ComponentPortType*	pOutPort = ipPorts[OMX_PORT_OUTPUTPORT_INDEX];

    if (!iPVCapabilityFlags.iOMXComponentUsesFullAVCFrames || !ipAvcEncoderObject->GetSpsPpsHeaderFlag())
    {
        if (iPVCapabilityFlags.iOMXComponentUsesNALStartCodes && ipOutputBuffer->nFilledLen == 4)
        {
            ipOutputBuffer->nFilledLen = 0;
        }


        //Attach the end of frame flag while sending out the last piece of output buffer
        if (iEndofStream)
        {
            ipOutputBuffer->nFlags |= OMX_BUFFERFLAG_EOS;
        }
        else
        {
            ipOutputBuffer->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;
        }

        if (OMX_TRUE == iSyncFlag)
        {
            ipOutputBuffer->nFlags |= OMX_BUFFERFLAG_SYNCFRAME;
            iSyncFlag = OMX_FALSE;
        }
        ReturnOutputBuffer(ipOutputBuffer, pOutPort);
    }
    else /* append extra data structure to buffer if iOMXComponentUsesFullAVCFrames is set and the buffer does not contain SPS or PPS NALs */
    {
        OMX_U32 CurrNALSize = ipOutputBuffer->nFilledLen - iNALSizeSum;
        if (CurrNALSize > 0)
        {
            if (iPVCapabilityFlags.iOMXComponentUsesNALStartCodes && !iEndOfOutputFrame && !iEndofStream)
            {
                oscl_memcpy(ipOutputBuffer->pBuffer + ipOutputBuffer->nOffset + ipOutputBuffer->nFilledLen, &NAL_START_CODE, sizeof(uint8) * 4);
                ipOutputBuffer->nFilledLen += 4;
                iNALSizeSum += 4;
            }

            iNALSizeArray[iNALCount] = CurrNALSize;
            iNALSizeSum += iNALSizeArray[iNALCount];
            iNALCount++;
        }

        if (iEndOfOutputFrame || iEndofStream)
        {
            if (!iPVCapabilityFlags.iOMXComponentUsesNALStartCodes)
            {
                if (OMX_FALSE == AppendExtraDataToBuffer(ipOutputBuffer, (OMX_EXTRADATATYPE) OMX_ExtraDataNALSizeArray, (OMX_U8*) iNALSizeArray, sizeof(uint32) * iNALCount))
                {
                    ipOutputBuffer->nFlags |= OMX_BUFFERFLAG_DATACORRUPT;

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                    (0, "OmxComponentAvcEncAO::ManageFrameBoundaries()  - Error appending extra data to Buffer 0x%x, TS=%d, returning anyway with data corrupt flag", ipOutputBuffer->pBuffer, iOutputTimeStamp));
                }
            }
            else if (0 == iNALCount)
            {
                iNALSizeSum = 0;
                ipOutputBuffer->nFilledLen = 0;
            }

            //Attach the end of frame flag while sending out the last piece of output buffer
            if (iEndOfOutputFrame)
            {
                ipOutputBuffer->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;
            }

            //Attach the end of stream flag
            if (iEndofStream)
            {
                ipOutputBuffer->nFlags |= OMX_BUFFERFLAG_EOS;
            }

            if (OMX_TRUE == iSyncFlag)
            {
                ipOutputBuffer->nFlags |= OMX_BUFFERFLAG_SYNCFRAME;
                iSyncFlag = OMX_FALSE;
            }

            ReturnOutputBuffer(ipOutputBuffer, pOutPort);

            oscl_memset((void *)iNALSizeArray, 0, iNALCount * sizeof(int32));
            iNALCount = 0;
            iNALSizeSum = 0;

            iEndOfOutputFrame = OMX_FALSE;
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_NOTICE,
                    (0, "OmxComponentAvcEncAO::ManageFrameBoundaries() Out"));
}

