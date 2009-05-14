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
/**
	@file pv_omxcomponent.h
	OpenMax decoder_component base header file.

*/

#ifndef PV_OMXCOMPONENT_H_INCLUDED
#define PV_OMXCOMPONENT_H_INCLUDED

#ifndef OMX_Component_h
#include "OMX_Component.h"
#endif


#ifndef OSCL_SCHEDULER_H_INCLUDED
#include "oscl_scheduler.h"
#endif

#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif

#ifndef PV_OMX_QUEUE_H_INCLUDED
#include "pv_omx_queue.h"
#endif

#ifndef PV_OMXCORE_H_INCLUDED
#include "pv_omxcore.h"
#endif

#define OMX_PORT_INPUTPORT_INDEX OMX_DirInput
#define OMX_PORT_OUTPUTPORT_INDEX OMX_DirOutput
#define OMX_PORT_ALLPORT_INDEX -1

#define OMX_PORT_NUMBER_FORMATS_SUPPORTED 4


/* Application's private data */
typedef struct ComponentPrivateType
{
    OMX_HANDLETYPE CompHandle;

}ComponentPrivateType;

/**
 * This is the Component template from which all
 * other Component instances are factored by the core.
 */
/**
 * The structure for port Type.
 */
typedef struct ComponentPortType
{
    /** @param pBuffer An array of pointers to buffer headers. */
    OMX_BUFFERHEADERTYPE** pBuffer;
    /** @param BufferState The State of the Buffer whether assigned or allocated */
    OMX_U32* BufferState;
    /** @param NumAssignedBuffers Number of buffer assigned on each port */
    OMX_U32 NumAssignedBuffers;
    /** @param pBufferQueue queue for buffer to be processed by the port */
    QueueType* pBufferQueue;
    OMX_STATETYPE TransientState;
    /** @param BufferUnderProcess  Boolean variables indicate whether the port is processing any buffer */
    OMX_BOOL BufferUnderProcess;
    OMX_PARAM_PORTDEFINITIONTYPE PortParam;
    /** @param NumBufferFlushed Number of buffer Flushed */
    OMX_U32 NumBufferFlushed;
    /** @param IsPortFlushed Boolean variables indicate port is being flushed at the moment */
    OMX_BOOL IsPortFlushed;

    //AUDIO SPECIFIC PARAMETERS
    OMX_AUDIO_PARAM_PORTFORMATTYPE	AudioParam;
    OMX_AUDIO_PARAM_PCMMODETYPE		AudioPcmMode;
    OMX_AUDIO_PARAM_WMATYPE			AudioWmaParam;
    OMX_AUDIO_PARAM_MP3TYPE			AudioMp3Param;
    OMX_AUDIO_CONFIG_EQUALIZERTYPE	AudioEqualizerType;
    OMX_AUDIO_PARAM_AACPROFILETYPE	AudioAacParam;
    OMX_AUDIO_PARAM_AMRTYPE			AudioAmrParam;

    //VIDEO SPECIFIC PARAMETERS
    OMX_VIDEO_PARAM_PORTFORMATTYPE	 VideoParam[OMX_PORT_NUMBER_FORMATS_SUPPORTED];
    OMX_VIDEO_PARAM_PROFILELEVELTYPE ProfileLevel;
    OMX_VIDEO_PARAM_MPEG4TYPE		 VideoMpeg4;
    OMX_VIDEO_PARAM_H263TYPE		 VideoH263;
    OMX_VIDEO_PARAM_AVCTYPE			 VideoAvc;
    OMX_VIDEO_PARAM_WMVTYPE			 VideoWmv;

    //VIDEO ENCODER SPECIFIC PARAMETERS
    OMX_CONFIG_ROTATIONTYPE				VideoOrientationType;
    OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE VideoErrorCorrection;
    OMX_VIDEO_PARAM_BITRATETYPE			VideoRateType;		//Only for rate control type in mpeg4
    OMX_VIDEO_PARAM_QUANTIZATIONTYPE	VideoQuantType;
    OMX_VIDEO_PARAM_VBSMCTYPE			VideoBlockMotionSize;
    OMX_VIDEO_PARAM_MOTIONVECTORTYPE	VideoMotionVector;
    OMX_VIDEO_PARAM_INTRAREFRESHTYPE	VideoIntraRefresh;
    OMX_VIDEO_PARAM_AVCSLICEFMO			AvcSliceFMO;

    OMX_CONFIG_FRAMERATETYPE			VideoConfigFrameRateType;
    OMX_VIDEO_CONFIG_BITRATETYPE		VideoConfigBitRateType;

    //This will be used to dynamically request the Iframe using SetConfig API
    OMX_CONFIG_INTRAREFRESHVOPTYPE		VideoIFrame;

    OMX_U32  ActualNumPortFormatsSupported;

} ComponentPortType;



class OmxComponentBase : public OsclActiveObject
{
    public:

        OmxComponentBase();
        virtual ~OmxComponentBase() {}

        /** Component entry points declarations without proxy interface*/
        static OMX_ERRORTYPE BaseComponentGetComponentVersion(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_OUT OMX_STRING pComponentName,
            OMX_OUT OMX_VERSIONTYPE* pComponentVersion,
            OMX_OUT OMX_VERSIONTYPE* pSpecVersion,
            OMX_OUT OMX_UUIDTYPE* pComponentUUID);

        static OMX_ERRORTYPE BaseComponentGetConfig(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_INDEXTYPE nIndex,
            OMX_INOUT OMX_PTR pComponentConfigStructure);

        static OMX_ERRORTYPE BaseComponentSetConfig(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_INDEXTYPE nIndex,
            OMX_IN  OMX_PTR pComponentConfigStructure);

        static OMX_ERRORTYPE BaseComponentGetExtensionIndex(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_STRING cParameterName,
            OMX_OUT OMX_INDEXTYPE* pIndexType);

        static OMX_ERRORTYPE BaseComponentGetState(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_OUT OMX_STATETYPE* pState);

        static OMX_ERRORTYPE BaseComponentGetParameter(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_INDEXTYPE nParamIndex,
            OMX_INOUT OMX_PTR ComponentParameterStructure);

        static OMX_ERRORTYPE BaseComponentSetParameter(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_INDEXTYPE nParamIndex,
            OMX_IN  OMX_PTR ComponentParameterStructure);

        static OMX_ERRORTYPE BaseComponentUseBuffer(
            OMX_IN OMX_HANDLETYPE hComponent,
            OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
            OMX_IN OMX_U32 nPortIndex,
            OMX_IN OMX_PTR pAppPrivate,
            OMX_IN OMX_U32 nSizeBytes,
            OMX_IN OMX_U8* pBuffer);

        static OMX_ERRORTYPE BaseComponentAllocateBuffer(
            OMX_IN OMX_HANDLETYPE hComponent,
            OMX_INOUT OMX_BUFFERHEADERTYPE** pBuffer,
            OMX_IN OMX_U32 nPortIndex,
            OMX_IN OMX_PTR pAppPrivate,
            OMX_IN OMX_U32 nSizeBytes);

        static OMX_ERRORTYPE BaseComponentFreeBuffer(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_U32 nPortIndex,
            OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer);

        static OMX_ERRORTYPE BaseComponentSendCommand(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_COMMANDTYPE Cmd,
            OMX_IN  OMX_U32 nParam,
            OMX_IN  OMX_PTR pCmdData);

        static OMX_ERRORTYPE BaseComponentComponentDeInit(
            OMX_IN  OMX_HANDLETYPE hComponent);

        static OMX_ERRORTYPE BaseComponentEmptyThisBuffer(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer);

        static OMX_ERRORTYPE BaseComponentFillThisBuffer(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer);

        static OMX_ERRORTYPE BaseComponentSetCallbacks(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_CALLBACKTYPE* pCallbacks,
            OMX_IN  OMX_PTR pAppData);

        /** Component entry points declarations with proxy interface*/
#if PROXY_INTERFACE
        static OMX_ERRORTYPE BaseComponentProxyGetComponentVersion(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_OUT OMX_STRING pComponentName,
            OMX_OUT OMX_VERSIONTYPE* pComponentVersion,
            OMX_OUT OMX_VERSIONTYPE* pSpecVersion,
            OMX_OUT OMX_UUIDTYPE* pComponentUUID);

        static OMX_ERRORTYPE BaseComponentProxyGetParameter(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_INDEXTYPE nParamIndex,
            OMX_INOUT OMX_PTR ComponentParameterStructure);

        static OMX_ERRORTYPE BaseComponentProxySetParameter(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_INDEXTYPE nParamIndex,
            OMX_IN  OMX_PTR ComponentParameterStructure);

        static OMX_ERRORTYPE BaseComponentProxyGetConfig(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_INDEXTYPE nIndex,
            OMX_INOUT OMX_PTR pComponentConfigStructure);

        static OMX_ERRORTYPE BaseComponentProxySetConfig(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_INDEXTYPE nIndex,
            OMX_IN  OMX_PTR pComponentConfigStructure);

        static OMX_ERRORTYPE BaseComponentProxyGetExtensionIndex(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_STRING cParameterName,
            OMX_OUT OMX_INDEXTYPE* pIndexType);

        static OMX_ERRORTYPE BaseComponentProxyGetState(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_OUT OMX_STATETYPE* pState);

        static OMX_ERRORTYPE BaseComponentProxyUseBuffer(
            OMX_IN OMX_HANDLETYPE hComponent,
            OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
            OMX_IN OMX_U32 nPortIndex,
            OMX_IN OMX_PTR pAppPrivate,
            OMX_IN OMX_U32 nSizeBytes,
            OMX_IN OMX_U8* pBuffer);

        static OMX_ERRORTYPE BaseComponentProxyAllocateBuffer(
            OMX_IN OMX_HANDLETYPE hComponent,
            OMX_INOUT OMX_BUFFERHEADERTYPE** pBuffer,
            OMX_IN OMX_U32 nPortIndex,
            OMX_IN OMX_PTR pAppPrivate,
            OMX_IN OMX_U32 nSizeBytes);

        static OMX_ERRORTYPE BaseComponentProxyFreeBuffer(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_U32 nPortIndex,
            OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer);

        static OMX_ERRORTYPE BaseComponentProxySendCommand(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_COMMANDTYPE Cmd,
            OMX_IN  OMX_U32 nParam,
            OMX_IN  OMX_PTR pCmdData);

        static OMX_ERRORTYPE BaseComponentProxyComponentDeInit(
            OMX_IN  OMX_HANDLETYPE hComponent);

        static OMX_ERRORTYPE BaseComponentProxyEmptyThisBuffer(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer);

        static OMX_ERRORTYPE BaseComponentProxyFillThisBuffer(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer);

        static OMX_ERRORTYPE BaseComponentProxySetCallbacks(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_CALLBACKTYPE* pCallbacks,
            OMX_IN  OMX_PTR pAppData);

#endif // PROXY_INTERFACE

        /*NON STATIC COUNTERPARTS OF STATIC MEMBER API'S */

        //Pure virtual functions, definition to be written in derived class
        virtual OMX_ERRORTYPE GetParameter(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_INDEXTYPE nParamIndex,
            OMX_INOUT OMX_PTR ComponentParameterStructure) = 0;

        virtual OMX_ERRORTYPE SetParameter(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_INDEXTYPE nParamIndex,
            OMX_IN  OMX_PTR ComponentParameterStructure) = 0;

        virtual OMX_ERRORTYPE GetConfig(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_INDEXTYPE nIndex,
            OMX_INOUT OMX_PTR pComponentConfigStructure);

        //Making Setconfig as virtual function to be implemented in respective component class
        virtual OMX_ERRORTYPE SetConfig(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_INDEXTYPE nIndex,
            OMX_IN  OMX_PTR pComponentConfigStructure);

        OMX_ERRORTYPE GetExtensionIndex(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_STRING cParameterName,
            OMX_OUT OMX_INDEXTYPE* pIndexType);

        void GetState(OMX_OUT OMX_STATETYPE* pState);

        OMX_ERRORTYPE UseBuffer(
            OMX_IN OMX_HANDLETYPE hComponent,
            OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
            OMX_IN OMX_U32 nPortIndex,
            OMX_IN OMX_PTR pAppPrivate,
            OMX_IN OMX_U32 nSizeBytes,
            OMX_IN OMX_U8* pBuffer);

        OMX_ERRORTYPE AllocateBuffer(
            OMX_IN OMX_HANDLETYPE hComponent,
            OMX_INOUT OMX_BUFFERHEADERTYPE** pBuffer,
            OMX_IN OMX_U32 nPortIndex,
            OMX_IN OMX_PTR pAppPrivate,
            OMX_IN OMX_U32 nSizeBytes);

        OMX_ERRORTYPE FreeBuffer(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_U32 nPortIndex,
            OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer);

        OMX_ERRORTYPE SendCommand(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_COMMANDTYPE Cmd,
            OMX_IN  OMX_S32 nParam,
            OMX_IN  OMX_PTR pCmdData);

        OMX_ERRORTYPE EmptyThisBuffer(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer);

        OMX_ERRORTYPE FillThisBuffer(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer);

        OMX_ERRORTYPE SetCallbacks(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_CALLBACKTYPE* pCallbacks,
            OMX_IN  OMX_PTR pAppData);


        OMX_PTR GetOmxHandle()
        {
            return &iOmxComponent;
        };

        /*OTHER PROCESSING FUNCTIONS */

        //Pure virtual function called from base, must have a definition in derived components
        //virtual void Decode() = 0;
        virtual void ProcessData() = 0;

        virtual void BufferMgmtFunction();
        virtual OMX_BOOL BufferMgmtWithoutMarker();
        //Extra routines called from BufferMgmtWithoutMarker that may vary from component to component
        virtual void ComponentBufferMgmtWithoutMarker();
        void TempInputBufferMgmtWithoutMarker();


        virtual void ProcessInBufferFlag() {};

        /* This function will exist only for audio components, thats why can't be made puire virtual
           Also putting it into the audio base class will require a check everytime in the
           BufferMgmtFunction() to create either an audio or video pointer to access this function
        */
        virtual void SyncWithInputTimestamp() {};
        virtual void ResetComponent() {};
        virtual OMX_ERRORTYPE ReAllocatePartialAssemblyBuffers(OMX_BUFFERHEADERTYPE* aInputBufferHdr)
        {
            OSCL_UNUSED_ARG(aInputBufferHdr);
            return OMX_ErrorNone;
        }

        OMX_BOOL AssemblePartialFrames(OMX_BUFFERHEADERTYPE* aInputBuffer);
        virtual OMX_BOOL ParseFullAVCFramesIntoNALs(OMX_BUFFERHEADERTYPE* aInputBuffer);
        OMX_ERRORTYPE MessageHandler(CoreMessage* Message);
        OMX_ERRORTYPE DoStateSet(OMX_U32);

        OMX_ERRORTYPE DisablePort(OMX_S32 PortIndex);
        void DisableSinglePort(OMX_U32 PortIndex);

        OMX_ERRORTYPE EnablePort(OMX_S32 PortIndex);
        void EnableSinglePort(OMX_U32 PortIndex);

        OMX_ERRORTYPE FlushPort(OMX_S32 PortIndex);
        void SetPortFlushFlag(OMX_S32, OMX_S32 index, OMX_BOOL value);
        void SetNumBufferFlush(OMX_S32, OMX_S32 index, OMX_S32 value);

        void ReturnInputBuffer(OMX_BUFFERHEADERTYPE* pInputBuffer, ComponentPortType* pPort);
        void ReturnOutputBuffer(OMX_BUFFERHEADERTYPE* pOutputBuffer, ComponentPortType* pPort);

        virtual OMX_ERRORTYPE ComponentInit() = 0;
        virtual OMX_ERRORTYPE ComponentDeInit() = 0;

        OMX_ERRORTYPE ConstructBaseComponent(OMX_PTR pAppData);
        OMX_ERRORTYPE DestroyBaseComponent();

        OMX_ERRORTYPE TunnelRequest(
            OMX_IN  OMX_HANDLETYPE hComp,
            OMX_IN  OMX_U32 nPort,
            OMX_IN  OMX_HANDLETYPE hTunneledComp,
            OMX_IN  OMX_U32 nTunneledPort,
            OMX_INOUT  OMX_TUNNELSETUPTYPE* pTunnelSetup);

        OMX_ERRORTYPE ParameterSanityCheck(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_U32 nPortIndex,
            OMX_IN  OMX_PTR pStructure,
            OMX_IN  size_t size);

        void SetHeader(OMX_PTR aheader, OMX_U32 asize);
        OMX_ERRORTYPE CheckHeader(OMX_PTR aheader, OMX_U32 asize);

        void ResetAfterFlush(OMX_S32 PortIndex);

        //Flag to call BufferMgmtFunction in the RunL() when the component state is executing
        OMX_BOOL				iBufferExecuteFlag;
        ComponentPrivateType*	ipAppPriv;


    protected:

        PVLogger* iLogger;

        void Run();

        OMX_CALLBACKTYPE*	ipCallbacks;
        OMX_PTR				iCallbackData;
        OMX_STATETYPE		iState;

        CoreDescriptorType* ipCoreDescriptor;
        OMX_U32				iNumInputBuffer;

        OMX_U8*				ipFrameDecodeBuffer;
        OMX_BOOL			iPartialFrameAssembly;
        OMX_BOOL			iIsInputBufferEnded;
        OMX_BOOL			iEndofStream;
        OMX_U8*				ipTempInputBuffer;
        OMX_U32				iTempInputBufferLength;
        OMX_COMPONENTTYPE*	ipTargetComponent;
        OMX_PTR				iTargetMarkData;
        OMX_COMPONENTTYPE*	ipTempTargetComponent;
        OMX_PTR				iTempTargetMarkData;
        OMX_BOOL			iMarkPropagate;
        OMX_BOOL			iNewInBufferRequired;
        OMX_BOOL			iNewOutBufRequired;
        OMX_U32				iTempConsumedLength;
        OMX_U32				iOutBufferCount;
        OMX_BOOL			iCodecReady;
        OMX_U8*				ipInputCurrBuffer;
        OMX_U32				iInputCurrBufferSize;
        OMX_U32				iInputCurrLength;
        OMX_S32				iFrameCount;
        OMX_BOOL			iStateTransitionFlag;

        OMX_BOOL				iEndOfFrameFlag;
        OMX_BUFFERHEADERTYPE*	ipInputBuffer;
        OMX_BUFFERHEADERTYPE*	ipOutputBuffer;
        OMX_U32					iOutputFrameLength;
        OMX_COMPONENTTYPE		iOmxComponent;	// structure
        OMX_U32					iNumPorts;
        OMX_U32					iCompressedFormatPortNum;
        OMX_PTR					ipComponentProxy;

        PV_OMXComponentCapabilityFlagsType iPVCapabilityFlags;

        //The ports of the component
        ComponentPortType** ipPorts;
        //Indicate whether component has been already initialized */
        OMX_BOOL iIsInit;
        //OpenMAX standard parameter that contains a short description of the available ports
        OMX_PORT_PARAM_TYPE iPortTypesParam;
        OMX_U32 iGroupPriority;
        //ID of a group of components that share the same logical chain
        OMX_U32 iGroupID;
        //Roles of the component
        OMX_U8 iComponentRole[OMX_MAX_STRINGNAME_SIZE];
        //Flag to indicate whether role has been set by client or not
        OMX_BOOL iComponentRoleFlag;
        //This field holds the private data associated with a mark request, if any
        OMX_MARKTYPE* ipMark;

        OMX_BOOL				iEosProcessing;
        OMX_BOOL				iFirstFragment;
        OMX_TICKS				iFrameTimestamp;
        OMX_BOOL				iRepositionFlag;
        OMX_U32					iSamplesPerFrame;
        OMX_BOOL				iSilenceInsertionInProgress;
        OMX_U32					iSilenceFramesNeeded;
        OMX_U32					iOutputMilliSecPerFrame;
        OMX_BOOL				iIsFirstOutputFrame;
        OMX_S32					iInputBufferRemainingBytes;
        OMX_BOOL				iResizePending;
        OMX_U8*					ipTempOutBufferForPortReconfig;
        OMX_U32					iSizeOutBufferForPortReconfig;
        OMX_BOOL				iSendOutBufferAfterPortReconfigFlag;
        OMX_TICKS				iTimestampOutBufferForPortReconfig;


        OMX_BOOL				iIsAudioComponent;


};


class OmxComponentAudio : public OmxComponentBase
{
    public:
        OmxComponentAudio();
        virtual ~OmxComponentAudio() {}

        OMX_ERRORTYPE GetParameter(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_INDEXTYPE nParamIndex,
            OMX_INOUT OMX_PTR ComponentParameterStructure);

        OMX_ERRORTYPE SetParameter(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_INDEXTYPE nParamIndex,
            OMX_IN  OMX_PTR ComponentParameterStructure);

        virtual void UpdateAACPlusFlag(OMX_BOOL aAacPlusFlag)
        {
            OSCL_UNUSED_ARG(aAacPlusFlag);
        }
};



class OmxComponentVideo : public OmxComponentBase
{
    public:
        OmxComponentVideo();
        virtual ~OmxComponentVideo() {}

        OMX_ERRORTYPE GetParameter(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_INDEXTYPE nParamIndex,
            OMX_INOUT OMX_PTR ComponentParameterStructure);

        OMX_ERRORTYPE SetParameter(
            OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_INDEXTYPE nParamIndex,
            OMX_IN  OMX_PTR ComponentParameterStructure);


};


#endif		//#ifndef PV_OMXCOMPONENT_H_INCLUDED
