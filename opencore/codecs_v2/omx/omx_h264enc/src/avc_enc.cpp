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

#include "avc_enc.h"


#define MAX_SUPPORTED_LAYER 1

/* global static functions */

void CbAvcEncDebugLog_OMX(uint32 *userData, AVCLogType type, char *string1, int val1, int val2)
{
    OSCL_UNUSED_ARG(userData);
    OSCL_UNUSED_ARG(type);
    OSCL_UNUSED_ARG(string1);
    OSCL_UNUSED_ARG(val1);
    OSCL_UNUSED_ARG(val2);

    return;
}

int CbAvcEncMalloc_OMX(void *userData, int32 size, int attribute)
{
    OSCL_UNUSED_ARG(userData);
    OSCL_UNUSED_ARG(attribute);

    uint8 *mem;

    mem = (uint8*) oscl_malloc(size);

    return (int)mem;
}

void CbAvcEncFree_OMX(void *userData, int mem)
{
    OSCL_UNUSED_ARG(userData);

    oscl_free((void*)mem);

    return;
}


int CbAvcEncDPBAlloc_OMX(void *userData, uint frame_size_in_mbs, uint num_buffers)
{
    AvcEncoder_OMX* pAvcEnc = (AvcEncoder_OMX*) userData;

    return pAvcEnc->AVC_DPBAlloc(frame_size_in_mbs, num_buffers);
}


void CbAvcEncFrameUnbind_OMX(void *userData, int indx)
{
    AvcEncoder_OMX* pAvcEnc = (AvcEncoder_OMX*) userData;

    pAvcEnc->AVC_FrameUnbind(indx);

    return;
}

int CbAvcEncFrameBind_OMX(void *userData, int indx, uint8 **yuv)
{
    AvcEncoder_OMX* pAvcEnc = (AvcEncoder_OMX*) userData;

    return pAvcEnc->AVC_FrameBind(indx, yuv);
}



//Class constructor function
AvcEncoder_OMX::AvcEncoder_OMX()
{
    iInitialized = OMX_FALSE;
    iSpsPpsHeaderFlag = OMX_FALSE;
    iReadyForNextFrame = OMX_TRUE;

    oscl_memset((void *)&iAvcHandle, 0, sizeof(AVCHandle));

    iAvcHandle.CBAVC_DPBAlloc = &CbAvcEncDPBAlloc_OMX;
    iAvcHandle.CBAVC_FrameBind = &CbAvcEncFrameBind_OMX;
    iAvcHandle.CBAVC_FrameUnbind = &CbAvcEncFrameUnbind_OMX;
    iAvcHandle.CBAVC_Free = &CbAvcEncFree_OMX;
    iAvcHandle.CBAVC_Malloc = &CbAvcEncMalloc_OMX;
    iAvcHandle.CBAVC_DebugLog = &CbAvcEncDebugLog_OMX;
    iAvcHandle.userData = this;

    ccRGBtoYUV = NULL;
    iYUVIn = NULL;
    iFramePtr = NULL;
    iDPB = NULL;
    iFrameUsed = NULL;
}


//Class destructor function
AvcEncoder_OMX::~AvcEncoder_OMX()
{
    //In case encoder cleanup has not been done yet, do it here
    AvcEncDeinit();
}


/* Encoder Initialization routine */
OMX_ERRORTYPE AvcEncoder_OMX::AvcEncInit(OMX_VIDEO_PORTDEFINITIONTYPE aInputParam,
        OMX_CONFIG_ROTATIONTYPE aInputOrientationType,
        OMX_VIDEO_PORTDEFINITIONTYPE aEncodeParam,
        OMX_VIDEO_PARAM_AVCTYPE aEncodeAvcParam,
        OMX_VIDEO_PARAM_BITRATETYPE aRateControlType,
        OMX_VIDEO_PARAM_QUANTIZATIONTYPE aQuantType,
        OMX_VIDEO_PARAM_MOTIONVECTORTYPE aSearchRange,
        OMX_VIDEO_PARAM_INTRAREFRESHTYPE aIntraRefresh,
        OMX_VIDEO_PARAM_VBSMCTYPE aVbsmcType)
{

    AVCEncParams aEncOption; /* encoding options */

    uint32* slice_group = NULL;
    int32 numTotalMBs;
    int32 Ysize16;
    int32 total_mb;
    int32 run_length;

    iSrcWidth = aInputParam.nFrameWidth;
    iSrcHeight = aInputParam.nFrameHeight;
    iFrameOrientation = aInputOrientationType.nRotation;


    if ((OMX_COLOR_FormatYUV420Planar == aInputParam.eColorFormat) ||
            (OMX_COLOR_Format24bitRGB888 == aInputParam.eColorFormat) ||
            (OMX_COLOR_Format12bitRGB444 == aInputParam.eColorFormat) ||
            (OMX_COLOR_FormatYUV420SemiPlanar == aInputParam.eColorFormat))
    {
        iVideoFormat = aInputParam.eColorFormat;
    }
    else
    {
        return OMX_ErrorUnsupportedSetting;
    }


    if (OMX_TRUE == iInitialized)
    {
        /* clean up before re-initialized */
        PVAVCCleanUpEncoder(&iAvcHandle);
        iAvcHandle.AVCObject = NULL;

        if (iYUVIn)
        {
            oscl_free(iYUVIn);
            iYUVIn = NULL;
        }

    }

    //Verify the input compression format
    if (OMX_VIDEO_CodingUnused != aInputParam.eCompressionFormat)
    {
        //Input port must have no compression supported
        return OMX_ErrorUnsupportedSetting;
    }

    // allocate iYUVIn
    if ((iSrcWidth & 0xF) || (iSrcHeight & 0xF) || (OMX_COLOR_FormatYUV420Planar != iVideoFormat)) /* Not multiple of 16 */
    {
        iYUVIn = (uint8*) oscl_malloc(((((iSrcWidth + 15) >> 4) * ((iSrcHeight + 15) >> 4)) * 3) << 7);
        if (NULL == iYUVIn)
        {
            return OMX_ErrorInsufficientResources;
        }
    }

    /* Initialize the color conversion */
    if (OMX_COLOR_Format24bitRGB888 == iVideoFormat)
    {
        ccRGBtoYUV = CCRGB24toYUV420::New();
        ccRGBtoYUV->Init(iSrcWidth, iSrcHeight, iSrcWidth, iSrcWidth, iSrcHeight, ((iSrcWidth + 15) >> 4) << 4, (iFrameOrientation == 180 ? CCBOTTOM_UP : 0));
    }
    if (OMX_COLOR_Format12bitRGB444 == iVideoFormat)
    {
        ccRGBtoYUV = CCRGB12toYUV420::New();
        ccRGBtoYUV->Init(iSrcWidth, iSrcHeight, iSrcWidth, iSrcWidth, iSrcHeight, ((iSrcWidth + 15) >> 4) << 4, (iFrameOrientation == 180 ? CCBOTTOM_UP : 0));
    }
    if (OMX_COLOR_FormatYUV420SemiPlanar == iVideoFormat)
    {
        ccRGBtoYUV = CCYUV420SEMItoYUV420::New();
        ccRGBtoYUV->Init(iSrcWidth, iSrcHeight, iSrcWidth, iSrcWidth , iSrcHeight, ((iSrcWidth + 15) >> 4) << 4, (iFrameOrientation == 180 ? CCBOTTOM_UP : 0));
    }

    aEncOption.width = aEncodeParam.nFrameWidth;
    aEncOption.height = aEncodeParam.nFrameHeight;
    aEncOption.frame_rate = (uint32)(1000 * ((aEncodeParam.xFramerate >> 16) + (OsclFloat)(aEncodeParam.xFramerate & 0xFFFF) / (1 << 16)));

    switch (aRateControlType.eControlRate)
    {
        case OMX_Video_ControlRateDisable:
        {
            aEncOption.rate_control = AVC_OFF;
            aEncOption.bitrate = 48000; // default
        }
        break;

        //Both the below cases have same body
        case OMX_Video_ControlRateConstant:
        case OMX_Video_ControlRateVariable:
        {
            aEncOption.rate_control = AVC_ON;
            aEncOption.bitrate = aEncodeParam.nBitrate;
        }
        break;

        default:
            return OMX_ErrorUnsupportedSetting;
    }

    //Set the profile of encoder
    switch (aEncodeAvcParam.eProfile)
    {
        case OMX_VIDEO_AVCProfileBaseline:
        {
            aEncOption.profile = AVC_BASELINE;
        }
        break;

        case OMX_VIDEO_AVCProfileMain:
        {
            aEncOption.profile = AVC_MAIN;
        }
        break;

        case OMX_VIDEO_AVCProfileExtended:
        {
            aEncOption.profile = AVC_EXTENDED;
        }
        break;

        case OMX_VIDEO_AVCProfileHigh:
        {
            aEncOption.profile = AVC_HIGH;
        }
        break;

        case OMX_VIDEO_AVCProfileHigh10:
        {
            aEncOption.profile = AVC_HIGH10;
        }
        break;

        case OMX_VIDEO_AVCProfileHigh422:
        {
            aEncOption.profile = AVC_HIGH422;
        }
        break;

        case OMX_VIDEO_AVCProfileHigh444:
        {
            aEncOption.profile = AVC_HIGH444;
        }
        break;

        default:
        {
            aEncOption.profile = AVC_BASELINE;
        }
        break;
    }


    //Set the level of avc encoder
    switch (aEncodeAvcParam.eLevel)
    {
        case OMX_VIDEO_AVCLevel1:
        {
            aEncOption.level = AVC_LEVEL1;
        }
        break;
        case OMX_VIDEO_AVCLevel1b:
        {
            aEncOption.level = AVC_LEVEL1_B;
        }
        break;
        case OMX_VIDEO_AVCLevel11:
        {
            aEncOption.level = AVC_LEVEL1_1;
        }
        break;
        case OMX_VIDEO_AVCLevel12:
        {
            aEncOption.level = AVC_LEVEL1_2;
        }
        break;
        case OMX_VIDEO_AVCLevel13:
        {
            aEncOption.level = AVC_LEVEL1_3;
        }
        break;


        case OMX_VIDEO_AVCLevel2:
        {
            aEncOption.level = AVC_LEVEL2;
        }
        break;
        case OMX_VIDEO_AVCLevel21:
        {
            aEncOption.level = AVC_LEVEL2_1;
        }
        break;
        case OMX_VIDEO_AVCLevel22:
        {
            aEncOption.level = AVC_LEVEL2_2;
        }
        break;


        case OMX_VIDEO_AVCLevel3:
        {
            aEncOption.level = AVC_LEVEL3;
        }
        break;
        case OMX_VIDEO_AVCLevel31:
        {
            aEncOption.level = AVC_LEVEL3_1;
        }
        break;
        case OMX_VIDEO_AVCLevel32:
        {
            aEncOption.level = AVC_LEVEL3_2;
        }
        break;


        case OMX_VIDEO_AVCLevel4:
        {
            aEncOption.level = AVC_LEVEL4;
        }
        break;
        case OMX_VIDEO_AVCLevel41:
        {
            aEncOption.level = AVC_LEVEL4_1;
        }
        break;
        case OMX_VIDEO_AVCLevel42:
        {
            aEncOption.level = AVC_LEVEL4_2;
        }
        break;


        case OMX_VIDEO_AVCLevel5:
        {
            aEncOption.level = AVC_LEVEL5;
        }
        break;
        case OMX_VIDEO_AVCLevel51:
        {
            aEncOption.level = AVC_LEVEL5_1;
        }
        break;

        default:
        {
            aEncOption.level = AVC_LEVEL5_1;
        }
        break;
    }

    aEncOption.initQP = aQuantType.nQpP;

    aEncOption.init_CBP_removal_delay = 1600;	//Default value
    aEncOption.CPB_size = ((uint32)(2 * aEncOption.bitrate));
    if (OMX_VIDEO_PictureTypeI == aEncodeAvcParam.nAllowedPictureTypes) // I-only
    {
        aEncOption.idr_period = 1;
    }
    else
    {
        //IPPPPPPPPPP, no I frame for a long period of time
        if (0xFFFFFFFF == aEncodeAvcParam.nPFrames)
        {
            aEncOption.idr_period = -1;
        }
        else
        {
            aEncOption.idr_period = aEncodeAvcParam.nPFrames + 1;
        }
    }

    if ((OMX_VIDEO_IntraRefreshCyclic == aIntraRefresh.eRefreshMode) ||
            (OMX_VIDEO_IntraRefreshBoth == aIntraRefresh.eRefreshMode))
    {
        aEncOption.intramb_refresh = aIntraRefresh.nCirMBs;
    }


    if ((OMX_VIDEO_IntraRefreshAdaptive == aIntraRefresh.eRefreshMode) ||
            (OMX_VIDEO_IntraRefreshBoth == aIntraRefresh.eRefreshMode))
    {
        aEncOption.auto_scd = AVC_ON;
    }
    else
    {
        aEncOption.auto_scd = AVC_OFF;
    }

    aEncOption.out_of_band_param_set = AVC_ON;		//Default value

    /* default values */
    aEncOption.poc_type = 2;
    aEncOption.log2_max_poc_lsb_minus_4 = 12;
    aEncOption.delta_poc_zero_flag = 0;
    aEncOption.offset_poc_non_ref = 0;
    aEncOption.offset_top_bottom  = 0;
    aEncOption.num_ref_in_cycle = 0;
    aEncOption.offset_poc_ref = NULL;

    aEncOption.num_ref_frame = 1;	//We only support this value

    //Since FMO is disabled in our case, num of slice group is always 1
#if (defined(TEST_FULL_AVC_FRAME_MODE) || defined(TEST_FULL_AVC_FRAME_MODE_SC))
    aEncOption.num_slice_group = 4;
    aEncOption.fmo_type = 6;
    aEncOption.use_overrun_buffer = AVC_OFF; // since we are to be outputing full frame buffers
#else
    aEncOption.num_slice_group = 1;
    aEncOption.fmo_type = 0;	//Disabled in this case
    aEncOption.use_overrun_buffer = AVC_ON;
#endif

    aEncOption.db_filter = AVC_ON;

    if (OMX_VIDEO_AVCLoopFilterEnable == aEncodeAvcParam.eLoopFilterMode)
    {
        aEncOption.disable_db_idc = 0;
    }
    else if (OMX_VIDEO_AVCLoopFilterDisable == aEncodeAvcParam.eLoopFilterMode)
    {
        aEncOption.disable_db_idc = 1;
    }
    else if (OMX_VIDEO_AVCLoopFilterDisableSliceBoundary == aEncodeAvcParam.eLoopFilterMode)
    {
        aEncOption.disable_db_idc = 2;
    }
    else
    {
        return OMX_ErrorUnsupportedSetting;
    }

    aEncOption.alpha_offset = 0;
    aEncOption.beta_offset = 0;
    aEncOption.constrained_intra_pred = (OMX_TRUE == aEncodeAvcParam.bconstIpred) ? AVC_ON : AVC_OFF;

    aEncOption.data_par = AVC_OFF;
    aEncOption.fullsearch = AVC_OFF;
    aEncOption.search_range = (aSearchRange.sXSearchRange <= aSearchRange.sYSearchRange ? aSearchRange.sXSearchRange : aSearchRange.sYSearchRange);

    if (OMX_Video_MotionVectorPixel == aSearchRange.eAccuracy)
    {
        aEncOption.sub_pel = AVC_OFF;
    }
    else
    {
        aEncOption.sub_pel = AVC_ON;
    }

    if (aVbsmcType.b16x16 != OMX_TRUE)
    {
        return OMX_ErrorBadParameter;
    }

    aEncOption.submb_pred =  AVC_OFF; // default for now, ignore aVbsmcType.8x16, 16x8, 8x8, etc.
    aEncOption.rdopt_mode = AVC_OFF;
    aEncOption.bidir_pred = AVC_OFF;

    Ysize16 = (((aEncOption.width + 15) >> 4) << 4) * (((aEncOption.height + 15) >> 4) << 4);
    numTotalMBs = Ysize16 >> 8;
    slice_group = (uint*) oscl_malloc(sizeof(uint) * numTotalMBs);

    int32 idx = 0;
    int32 ii;
    for (ii = 0; ii < numTotalMBs; ii++)
    {
        slice_group[ii] = idx++;
        if (idx >= aEncOption.num_slice_group)
        {
            idx = 0;
        }
    }
    aEncOption.slice_group = slice_group;

    if (aEncOption.num_slice_group > 1)  /* divide slice group equally */
    {
        run_length = numTotalMBs / aEncOption.num_slice_group;
        total_mb = 0;
        for (idx = 0; idx < aEncOption.num_slice_group; idx++)
        {
            aEncOption.run_length_minus1[idx] = run_length - 1;
            total_mb += run_length;
        }
        if (total_mb < numTotalMBs)
        {
            aEncOption.run_length_minus1[aEncOption.num_slice_group-1] += (numTotalMBs - total_mb);
        }
    }

    /***** Initialize the encoder *****/
    if (AVCENC_SUCCESS != PVAVCEncInitialize(&iAvcHandle, &aEncOption, NULL, NULL))
    {
        iInitialized = OMX_FALSE;
        return OMX_ErrorBadParameter;
    }

    iIDR = OMX_TRUE;
    iDispOrd = 0;
    iInitialized = OMX_TRUE;
    iReadyForNextFrame = OMX_TRUE;

    oscl_free(slice_group);

    return OMX_ErrorNone;

}


AVCEnc_Status AvcEncoder_OMX::AvcEncodeSendInput(OMX_U8*    aInBuffer,
        OMX_U32*   aInBufSize,
        OMX_TICKS  aInTimeStamp)
{
    AVCEnc_Status AvcStatus;

    if (OMX_COLOR_FormatYUV420Planar == iVideoFormat)
    {
        /* Input Buffer Size Check
         * Input buffer size should be equal to one frame, otherwise drop the frame
         * as it is a corrupt data and don't encode it */
        if (*aInBufSize < (OMX_U32)((iSrcWidth * iSrcHeight * 3) >> 1))
        {
            //Mark the inpur buffer consumed to indicate corrupt frame
            *aInBufSize = 0;
            return AVCENC_FAIL;
        }

        if (iYUVIn) /* iSrcWidth is not multiple of 4 or iSrcHeight is odd number */
        {
            CopyToYUVIn(aInBuffer, iSrcWidth, iSrcHeight,
                        ((iSrcWidth + 15) >> 4) << 4, ((iSrcHeight + 15) >> 4) << 4);
            iVideoIn = iYUVIn;
        }
        else /* otherwise, we can just use aVidIn->iSource */
        {
            iVideoIn = aInBuffer;
        }
    }
    else if (OMX_COLOR_Format12bitRGB444 == iVideoFormat)
    {
        if (*aInBufSize < (OMX_U32)(iSrcWidth * iSrcHeight * 2))
        {
            //Mark the inpur buffer consumed to indicate corrupt frame
            *aInBufSize = 0;
            return AVCENC_FAIL;
        }
        ccRGBtoYUV->Convert((uint8*)aInBuffer, iYUVIn);
        iVideoIn = iYUVIn;
    }
    else if (OMX_COLOR_Format24bitRGB888 == iVideoFormat)
    {
        if (*aInBufSize < (OMX_U32)(iSrcWidth * iSrcHeight * 3))
        {
            //Mark the inpur buffer consumed to indicate corrupt frame
            *aInBufSize = 0;
            return AVCENC_FAIL;
        }
        ccRGBtoYUV->Convert((uint8*)aInBuffer, iYUVIn);
        iVideoIn = iYUVIn;
    }
    else if (OMX_COLOR_FormatYUV420SemiPlanar == iVideoFormat)
    {
        if (*aInBufSize < (OMX_U32)(iSrcWidth * iSrcHeight * 3) >> 1)
        {
            //Mark the inpur buffer consumed to indicate corrupt frame
            *aInBufSize = 0;
            return AVCENC_FAIL;
        }
        ccRGBtoYUV->Convert((uint8*)aInBuffer, iYUVIn);
        iVideoIn = iYUVIn;
    }

    /* assign with backward-P or B-Vop this timestamp must be re-ordered */
    iTimeStamp = aInTimeStamp;

    iVidIn.height = ((iSrcHeight + 15) >> 4) << 4;
    iVidIn.pitch = ((iSrcWidth + 15) >> 4) << 4;
    iVidIn.coding_timestamp = iTimeStamp;
    iVidIn.YCbCr[0] = (uint8*)iVideoIn;
    iVidIn.YCbCr[1] = (uint8*)(iVideoIn + iVidIn.height * iVidIn.pitch);
    iVidIn.YCbCr[2] = iVidIn.YCbCr[1] + ((iVidIn.height * iVidIn.pitch) >> 2);
    iVidIn.disp_order = iDispOrd;

    AvcStatus = PVAVCEncSetInput(&iAvcHandle, &iVidIn);

    if (AVCENC_SUCCESS == AvcStatus)
    {
        iDispOrd++;
        iReadyForNextFrame = OMX_FALSE;
        return AvcStatus;
    }
    else if (AVCENC_NEW_IDR == AvcStatus)
    {
        iDispOrd++;
        iIDR = OMX_TRUE;
        iReadyForNextFrame = OMX_FALSE;
        return AvcStatus;
    }
    else
    {
        return AvcStatus;
    }

}


/*Encode routine */
AVCEnc_Status AvcEncoder_OMX::AvcEncodeVideo(OMX_U8* aOutBuffer,
        OMX_U32*   aOutputLength,
        OMX_BOOL*  aBufferOverRun,
        OMX_U8**   aOverBufferPointer,
        OMX_U8*    aInBuffer,
        OMX_U32*   aInBufSize,
        OMX_TICKS  aInTimeStamp,
        OMX_TICKS* aOutTimeStamp,
        OMX_BOOL*  aSyncFlag)
{
    AVCEnc_Status AvcStatus;
    AVCEnc_Status ReturnValue;
    AVCEnc_Status	SetInputStatus = AVCENC_SUCCESS;
    AVCFrameIO recon;
    uint Size;
    int  NALType = 0;

    if (OMX_FALSE == iSpsPpsHeaderFlag)
    {
        Size = *aOutputLength;
        AvcStatus =	PVAVCEncodeNAL(&iAvcHandle, aOutBuffer, &Size, &NALType);

        //No more SPS and PPS NAL's to come, mark the header flag as true
        if (AVCENC_WRONG_STATE == AvcStatus)
        {
            *aOutputLength = 0;
            iSpsPpsHeaderFlag = OMX_TRUE;
            return AVCENC_WRONG_STATE;
        }

        switch (NALType)
        {
            case AVC_NALTYPE_SPS:
            case AVC_NALTYPE_PPS:
            {
                *aOutputLength = Size;
                //Mark the SPS and PPS buffers with the first input bufer timestamp
                *aOutTimeStamp = aInTimeStamp;
                return AVCENC_SUCCESS;
            }

            default:
            {
                //No more SPS and PPS NAL's to come, mark the header flag as true
                *aOutputLength = 0;
                iSpsPpsHeaderFlag = OMX_TRUE;
                return AVCENC_FAIL;
            }
        }
    }


    if (OMX_TRUE == iReadyForNextFrame)
    {
        SetInputStatus = AvcEncodeSendInput(aInBuffer, aInBufSize, aInTimeStamp);
    }

    if ((AVCENC_SUCCESS == SetInputStatus) || (AVCENC_NEW_IDR == SetInputStatus))		//success
    {
        Size = *aOutputLength;

        AvcStatus = PVAVCEncodeNAL(&iAvcHandle, (uint8*)aOutBuffer, &Size, &NALType);

        if (AVCENC_SUCCESS == AvcStatus)
        {
            *aSyncFlag = iIDR;
            //Calling the overrun buffer api to see whether overrun buffer has been used or not
            *aOverBufferPointer = PVAVCEncGetOverrunBuffer(&iAvcHandle);

            if (NULL != *aOverBufferPointer)
            {
                //Output data has been filled in the over run buffer, mark the flag as true
                *aBufferOverRun = OMX_TRUE;
            }

            ReturnValue = AVCENC_SUCCESS;
        }
        else if (AVCENC_PICTURE_READY == AvcStatus)
        {
            //Calling the overrun buffer api to see whether overrun buffer has been used or not
            *aOverBufferPointer = PVAVCEncGetOverrunBuffer(&iAvcHandle);

            if (NULL != *aOverBufferPointer)
            {
                //Output data has been filled in the over run buffer, mark the flag as true
                *aBufferOverRun = OMX_TRUE;
            }

            *aSyncFlag = iIDR;
            if (iIDR == OMX_TRUE)
            {
                iIDR = OMX_FALSE;
            }

            ReturnValue = AVCENC_PICTURE_READY;

            iReadyForNextFrame = OMX_TRUE;	//ready to receive another set of input frame

            AvcStatus = PVAVCEncGetRecon(&iAvcHandle, &recon);
            if (AVCENC_SUCCESS == AvcStatus)
            {
                PVAVCEncReleaseRecon(&iAvcHandle, &recon);
            }
        }
        else if (AVCENC_SKIPPED_PICTURE == AvcStatus)
        {
            *aOutputLength = 0;
            iReadyForNextFrame = OMX_TRUE;	//ready to receive another set of input frame
            return AVCENC_SKIPPED_PICTURE;
        }
        else
        {
            *aOutputLength = 0;
            iReadyForNextFrame = OMX_TRUE;	//ready to receive another set of input frame
            return AVCENC_FAIL;
        }

        *aOutputLength = Size;
        *aOutTimeStamp = iTimeStamp;

        return ReturnValue;

    }
    else if (AVCENC_SKIPPED_PICTURE == SetInputStatus)
    {
        *aOutputLength = 0;
        return AVCENC_SKIPPED_PICTURE;
    }
    else
    {
        *aOutputLength = 0;
        return AVCENC_FAIL;
    }
}


//Deinitialize the avc encoder here and perform the cleanup and free operations
OMX_ERRORTYPE AvcEncoder_OMX::AvcEncDeinit()
{
    if (OMX_TRUE == iInitialized)
    {
        PVAVCCleanUpEncoder(&iAvcHandle);
        iAvcHandle.AVCObject = NULL;

        iInitialized = OMX_FALSE;
        iReadyForNextFrame = OMX_TRUE;

        if (iYUVIn)
        {
            oscl_free(iYUVIn);
            iYUVIn = NULL;
        }
    }

    if (iFrameUsed)
    {
        oscl_free(iFrameUsed);
        iFrameUsed = NULL;
    }

    if (ccRGBtoYUV)
    {
        OSCL_DELETE(ccRGBtoYUV);
        ccRGBtoYUV = NULL;
    }

    if (iDPB)
    {
        oscl_free(iDPB);
        iDPB = NULL;
    }

    if (iFramePtr)
    {
        oscl_free(iFramePtr);
        iFramePtr = NULL;
    }

    return OMX_ErrorNone;
}


int AvcEncoder_OMX::AVC_DPBAlloc(uint frame_size_in_mbs, uint num_buffers)
{
    int ii;
    uint frame_size = (frame_size_in_mbs << 8) + (frame_size_in_mbs << 7);

    if (iDPB) oscl_free(iDPB); // free previous one first

    iDPB = (uint8*) oscl_malloc(sizeof(uint8) * frame_size * num_buffers);
    if (iDPB == NULL)
    {
        return 0;
    }

    iNumFrames = num_buffers;

    if (iFrameUsed) oscl_free(iFrameUsed); // free previous one

    iFrameUsed = (bool*) oscl_malloc(sizeof(bool) * num_buffers);
    if (iFrameUsed == NULL)
    {
        return 0;
    }

    if (iFramePtr) oscl_free(iFramePtr); // free previous one
    iFramePtr = (uint8**) oscl_malloc(sizeof(uint8*) * num_buffers);
    if (iFramePtr == NULL)
    {
        return 0;
    }

    iFramePtr[0] = iDPB;
    iFrameUsed[0] = false;

    for (ii = 1; ii < (int)num_buffers; ii++)
    {
        iFrameUsed[ii] = false;
        iFramePtr[ii] = iFramePtr[ii-1] + frame_size;
    }

    return 1;
}


//Request for an I frame while encoding is in process
OMX_ERRORTYPE AvcEncoder_OMX::AvcRequestIFrame()
{
    if (AVCENC_SUCCESS != PVAVCEncIDRRequest(&iAvcHandle))
    {
        return OMX_ErrorUndefined;
    }

    return OMX_ErrorNone;

}

//Routine to update bitrate dynamically when encoding is in progress
OMX_BOOL AvcEncoder_OMX::AvcUpdateBitRate(OMX_U32 aEncodedBitRate)
{
    int32 BitRate[2] = {0, 0};
    BitRate[0] = aEncodedBitRate;

    if (AVCENC_SUCCESS != PVAVCEncUpdateBitRate(&iAvcHandle, BitRate[0]))
    {
        return OMX_FALSE;
    }

    return OMX_TRUE;
}

//Routine to update frame rate dynamically when encoding is in progress
OMX_BOOL AvcEncoder_OMX::AvcUpdateFrameRate(OMX_U32 aEncodeFramerate)
{
    OMX_U32 EncFrameRate[2] = {0, 0};
    EncFrameRate[0] = aEncodeFramerate >> 16;

    if (AVCENC_SUCCESS != PVAVCEncUpdateFrameRate(&iAvcHandle, (uint32)(1000*EncFrameRate[0]), 1000))
    {
        return OMX_FALSE;
    }

    return OMX_TRUE;

}



/* ///////////////////////////////////////////////////////////////////////// */
void AvcEncoder_OMX::AVC_FrameUnbind(int indx)
{
    /*if (indx < iNumFrames)
    {
    	iFrameUsed[indx] = false;
    }*/

    OSCL_UNUSED_ARG(indx);
    return ;
}

/* ///////////////////////////////////////////////////////////////////////// */
int AvcEncoder_OMX::AVC_FrameBind(int indx, uint8** yuv)
{
    /*if ((iFrameUsed[indx]==true) || (indx >= iNumFrames))
    {
    	return 0; // already in used
    }
    iFrameUsed[indx] = true;*/

    *yuv = iFramePtr[indx];
    return 1;
}



/* ///////////////////////////////////////////////////////////////////////// */
/* Copy from YUV input to YUV frame inside AvcEnc lib						*/
/* When input is not YUV, the color conv will write it directly to iVideoInOut. */
/* ///////////////////////////////////////////////////////////////////////// */

void AvcEncoder_OMX::CopyToYUVIn(uint8 *YUV, int width, int height, int width_16, int height_16)
{
    uint8 *y, *u, *v, *yChan, *uChan, *vChan;
    int y_ind, ilimit, jlimit, i, j, ioffset;
    int size = width * height;
    int size16 = width_16 * height_16;

    /* do padding at the bottom first */
    /* do padding if input RGB size(height) is different from the output YUV size(height_16) */
    if (height < height_16 || width < width_16) /* if padding */
    {
        int offset = (height < height_16) ? height : height_16;

        offset = (offset * width_16);

        if (width < width_16)
        {
            offset -= (width_16 - width);
        }

        yChan = (uint8*)(iYUVIn + offset);
        oscl_memset(yChan, 16, size16 - offset); /* pad with zeros */

        uChan = (uint8*)(iYUVIn + size16 + (offset >> 2));
        oscl_memset(uChan, 128, (size16 - offset) >> 2);

        vChan = (uint8*)(iYUVIn + size16 + (size16 >> 2) + (offset >> 2));
        oscl_memset(vChan, 128, (size16 - offset) >> 2);
    }

    /* then do padding on the top */
    yChan = (uint8*)iYUVIn; /* Normal order */
    uChan = (uint8*)(iYUVIn + size16);
    vChan = (uint8*)(uChan + (size16 >> 2));

    u = (uint8*)(&(YUV[size]));
    v = (uint8*)(&(YUV[size*5/4]));

    /* To center the output */
    if (height_16 > height)   /* output taller than input */
    {
        if (width_16 >= width)  /* output wider than or equal input */
        {
            i = ((height_16 - height) >> 1) * width_16 + (((width_16 - width) >> 3) << 2);
            /* make sure that (width_16-width)>>1 is divisible by 4 */
            j = ((height_16 - height) >> 2) * (width_16 >> 1) + (((width_16 - width) >> 4) << 2);
            /* make sure that (width_16-width)>>2 is divisible by 4 */
        }
        else  /* output narrower than input */
        {
            i = ((height_16 - height) >> 1) * width_16;
            j = ((height_16 - height) >> 2) * (width_16 >> 1);
            YUV += ((width - width_16) >> 1);
            u += ((width - width_16) >> 2);
            v += ((width - width_16) >> 2);
        }
        oscl_memset((uint8 *)yChan, 16, i);
        yChan += i;
        oscl_memset((uint8 *)uChan, 128, j);
        uChan += j;
        oscl_memset((uint8 *)vChan, 128, j);
        vChan += j;
    }
    else   /* output shorter or equal input */
    {
        if (width_16 >= width)   /* output wider or equal input */
        {
            i = (((width_16 - width) >> 3) << 2);
            /* make sure that (width_16-width)>>1 is divisible by 4 */
            j = (((width_16 - width) >> 4) << 2);
            /* make sure that (width_16-width)>>2 is divisible by 4 */
            YUV += (((height - height_16) >> 1) * width);
            u += (((height - height_16) >> 1) * width) >> 2;
            v += (((height - height_16) >> 1) * width) >> 2;
        }
        else  /* output narrower than input */
        {
            i = 0;
            j = 0;
            YUV += (((height - height_16) >> 1) * width + ((width - width_16) >> 1));
            u += (((height - height_16) >> 1) * width + ((width - width_16) >> 1)) >> 2;
            v += (((height - height_16) >> 1) * width + ((width - width_16) >> 1)) >> 2;
        }
        oscl_memset((uint8 *)yChan, 16, i);
        yChan += i;
        oscl_memset((uint8 *)uChan, 128, j);
        uChan += j;
        oscl_memset((uint8 *)vChan, 128, j);
        vChan += j;
    }

    /* Copy with cropping or zero-padding */
    if (height < height_16)
        jlimit = height;
    else
        jlimit = height_16;

    if (width < width_16)
    {
        ilimit = width;
        ioffset = width_16 - width;
    }
    else
    {
        ilimit = width_16;
        ioffset = 0;
    }

    /* Copy Y */
    /* Set up pointer for fast looping */
    y = (uint8*)YUV;

    if (width == width_16 && height == height_16) /* no need to pad */
    {
        oscl_memcpy(yChan, y, size);
    }
    else
    {
        for (y_ind = 0; y_ind < (jlimit - 1) ;y_ind++)
        {
            oscl_memcpy(yChan, y, ilimit);
            oscl_memset(yChan + ilimit, 16, ioffset); /* pad with zero */
            yChan += width_16;
            y += width;
        }
        oscl_memcpy(yChan, y, ilimit); /* last line no padding */
    }
    /* Copy U and V */
    /* Set up pointers for fast looping */
    if (width == width_16 && height == height_16) /* no need to pad */
    {
        oscl_memcpy(uChan, u, size >> 2);
        oscl_memcpy(vChan, v, size >> 2);
    }
    else
    {
        for (y_ind = 0; y_ind < (jlimit >> 1) - 1;y_ind++)
        {
            oscl_memcpy(uChan, u, ilimit >> 1);
            oscl_memcpy(vChan, v, ilimit >> 1);
            oscl_memset(uChan + (ilimit >> 1), 128, ioffset >> 1);
            oscl_memset(vChan + (ilimit >> 1), 128, ioffset >> 1);
            uChan += (width_16 >> 1);
            u += (width >> 1);
            vChan += (width_16 >> 1);
            v += (width >> 1);
        }
        oscl_memcpy(uChan, u, ilimit >> 1); /* last line no padding */
        oscl_memcpy(vChan, v, ilimit >> 1);
    }

    return ;
}

OMX_BOOL AvcEncoder_OMX::GetSpsPpsHeaderFlag()
{
    return iSpsPpsHeaderFlag;
}


