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
#include "oscl_types.h"
#include "avc_dec.h"
#include "avcdec_int.h"


/*************************************/
/* functions needed for video engine */
/*************************************/

/* These two functions are for callback functions of AvcHandle */
int32 CBAVC_Malloc_OMX(void* aUserData, int32 aSize, int32 aAttribute)
{
    OSCL_UNUSED_ARG(aUserData);
    OSCL_UNUSED_ARG(aAttribute);
    void* pPtr;

    pPtr = oscl_malloc(aSize);
    return (int32) pPtr;
}

void CBAVC_Free_OMX(void* aUserData, int32 aMem)
{
    OSCL_UNUSED_ARG(aUserData);
    oscl_free((uint8*) aMem);
}


AVCDec_Status CBAVCDec_GetData_OMX(void* aUserData, uint8** aBuffer, uint* aSize)
{
    OSCL_UNUSED_ARG(aUserData);
    OSCL_UNUSED_ARG(aBuffer);
    OSCL_UNUSED_ARG(aSize);
    return AVCDEC_FAIL;  /* nothing for now */
}

int32 AvcDecoder_OMX::AllocateBuffer_OMX(void* aUserData, int32 i, uint8** aYuvBuffer)
{
    AvcDecoder_OMX* pAvcDecoder_OMX = (AvcDecoder_OMX*)aUserData;

    if (NULL == pAvcDecoder_OMX)
    {
        return 0;
    }

    *aYuvBuffer = pAvcDecoder_OMX->pDpbBuffer + i * pAvcDecoder_OMX->FrameSize;
    //Store the input timestamp at the correct index
    pAvcDecoder_OMX->DisplayTimestampArray[i] = pAvcDecoder_OMX->CurrInputTimestamp;
    return 1;
}


void UnbindBuffer_OMX(void* aUserData, int32 i)
{
    OSCL_UNUSED_ARG(aUserData);
    OSCL_UNUSED_ARG(i);
    return;
}

int32 AvcDecoder_OMX::ActivateSPS_OMX(void* aUserData, uint aSizeInMbs, uint aNumBuffers)
{
    AvcDecoder_OMX* pAvcDecoder_OMX = (AvcDecoder_OMX*)aUserData;

    if (NULL == pAvcDecoder_OMX)
    {
        return 0;
    }

    PVAVCDecGetSeqInfo(&(pAvcDecoder_OMX->AvcHandle), &(pAvcDecoder_OMX->SeqInfo));

    if (pAvcDecoder_OMX->pDpbBuffer)
    {
        oscl_free(pAvcDecoder_OMX->pDpbBuffer);
        pAvcDecoder_OMX->pDpbBuffer = NULL;
    }

    pAvcDecoder_OMX->FrameSize = (aSizeInMbs << 7) * 3;
    pAvcDecoder_OMX->pDpbBuffer = (uint8*) oscl_malloc(aNumBuffers * (pAvcDecoder_OMX->FrameSize));

    return 1;
}

/* initialize video decoder */
OMX_BOOL AvcDecoder_OMX::InitializeVideoDecode_OMX()
{
    /* Initialize AvcHandle */
    AvcHandle.AVCObject = NULL;
    AvcHandle.userData = (void*)this;
    AvcHandle.CBAVC_DPBAlloc = ActivateSPS_OMX;
    AvcHandle.CBAVC_FrameBind = AllocateBuffer_OMX;
    AvcHandle.CBAVC_FrameUnbind = UnbindBuffer_OMX;
    AvcHandle.CBAVC_Malloc = CBAVC_Malloc_OMX;
    AvcHandle.CBAVC_Free = CBAVC_Free_OMX;

    return OMX_TRUE;
}

OMX_BOOL AvcDecoder_OMX::FlushOutput_OMX(OMX_U8* aOutBuffer, OMX_U32* aOutputLength, OMX_TICKS* aOutTimestamp, OMX_S32 OldWidth, OMX_S32 OldHeight)
{
    AVCFrameIO Output;
    AVCDec_Status Status;
    int32 Index, Release, FrameSize;
    OMX_S32 OldFrameSize = ((OldWidth + 15) & (~15)) * ((OldHeight + 15) & (~15));

    Output.YCbCr[0] = Output.YCbCr[1] = Output.YCbCr[2] = NULL;
    Status = PVAVCDecGetOutput(&(AvcHandle), &Index, &Release, &Output);

    if (Status == AVCDEC_FAIL)
    {
        return OMX_FALSE;
    }

    *aOutTimestamp = DisplayTimestampArray[Index];
    *aOutputLength = 0; // init to 0

    if (Output.YCbCr[0])
    {
        FrameSize = Output.pitch * Output.height;
        // it should not happen that the frame size is smaller than available buffer size, but check just in case
        if (FrameSize <= OldFrameSize)
        {
            *aOutputLength = (Output.pitch * Output.height * 3) >> 1;

            oscl_memcpy(aOutBuffer, Output.YCbCr[0], FrameSize);
            oscl_memcpy(aOutBuffer + FrameSize, Output.YCbCr[1], FrameSize >> 2);
            oscl_memcpy(aOutBuffer + FrameSize + FrameSize / 4, Output.YCbCr[2], FrameSize >> 2);
        }
        // else, the frame length is reported as zero, and there is no copying
    }


    return OMX_TRUE;
}


/* Initialization routine */
OMX_ERRORTYPE AvcDecoder_OMX::AvcDecInit_OMX()
{
    if (OMX_FALSE == InitializeVideoDecode_OMX())
    {
        return OMX_ErrorInsufficientResources;
    }

    //Set up the cleanup object in order to do clean up work automatically
    pCleanObject = OSCL_NEW(AVCCleanupObject_OMX, (&AvcHandle));

    return OMX_ErrorNone;
}


/*Decode routine */
OMX_BOOL AvcDecoder_OMX::AvcDecodeVideo_OMX(OMX_U8* aOutBuffer, OMX_U32* aOutputLength,
        OMX_U8** aInputBuf, OMX_U32* aInBufSize,
        OMX_PARAM_PORTDEFINITIONTYPE* aPortParam,
        OMX_S32* iFrameCount, OMX_BOOL aMarkerFlag, OMX_TICKS* aOutTimestamp, OMX_BOOL *aResizeFlag)
{
    AVCDec_Status Status;
    OMX_S32 Width, Height;
    OMX_S32 crop_top, crop_bottom, crop_right, crop_left;
    uint8* pNalBuffer;
    int32 NalSize, NalType, NalRefId;
    //int32 PicType;
    AVCDecObject* pDecVid;

    *aResizeFlag = OMX_FALSE;
    OMX_U32 OldWidth, OldHeight;

    OldWidth = 	aPortParam->format.video.nFrameWidth;
    OldHeight = aPortParam->format.video.nFrameHeight;


    if (!aMarkerFlag)
    {
        if (AVCDEC_FAIL == GetNextFullNAL_OMX(&pNalBuffer, &NalSize, *aInputBuf, aInBufSize))
        {
            Status = (AVCDec_Status) FlushOutput_OMX(aOutBuffer, aOutputLength, aOutTimestamp, OldWidth, OldHeight);

            if (AVCDEC_FAIL != Status)
            {
                return OMX_TRUE;
            }
            else
            {
                return OMX_FALSE;
            }
        }
    }
    else
    {
        pNalBuffer = *aInputBuf;
        NalSize = *aInBufSize;
        //Assuming that the buffer with marker bit contains one full NAL
        *aInBufSize = 0;
    }

    if (AVCDEC_FAIL == PVAVCDecGetNALType(pNalBuffer, NalSize, &NalType, &NalRefId))
    {
        return OMX_FALSE;
    }

    if (AVC_NALTYPE_SPS == (AVCNalUnitType)NalType)
    {
        if (PVAVCDecSeqParamSet(&(AvcHandle), pNalBuffer, NalSize) != AVCDEC_SUCCESS)
        {
            return OMX_FALSE;
        }

        pDecVid = (AVCDecObject*) AvcHandle.AVCObject;

        Width = (pDecVid->seqParams[0]->pic_width_in_mbs_minus1 + 1) * 16;
        Height = (pDecVid->seqParams[0]->pic_height_in_map_units_minus1 + 1) * 16;

        if (pDecVid->seqParams[0]->frame_cropping_flag)
        {
            crop_left = 2 * pDecVid->seqParams[0]->frame_crop_left_offset;
            crop_right = Width - (2 * pDecVid->seqParams[0]->frame_crop_right_offset + 1);

            if (pDecVid->seqParams[0]->frame_mbs_only_flag)
            {
                crop_top = 2 * pDecVid->seqParams[0]->frame_crop_top_offset;
                crop_bottom = Height - (2 * pDecVid->seqParams[0]->frame_crop_bottom_offset + 1);
            }
            else
            {
                crop_top = 4 * pDecVid->seqParams[0]->frame_crop_top_offset;
                crop_bottom = Height - (4 * pDecVid->seqParams[0]->frame_crop_bottom_offset + 1);
            }
        }
        else  /* no cropping flag, just give the first and last pixel */
        {
            crop_bottom = Height - 1;
            crop_right = Width - 1;
            crop_top = crop_left = 0;
        }

        aPortParam->format.video.nFrameWidth = crop_right - crop_left + 1;
        aPortParam->format.video.nFrameHeight = crop_bottom - crop_top + 1;

        //if( (OldWidth != aPortParam->format.video.nFrameWidth) || (OldHeight !=	aPortParam->format.video.nFrameHeight))
        // FORCE RESIZE ALWAYS FOR SPS
        *aResizeFlag = OMX_TRUE;

        (*iFrameCount)++;

    }

    else if (AVC_NALTYPE_PPS == (AVCNalUnitType) NalType)
    {
        if (PVAVCDecPicParamSet(&(AvcHandle), pNalBuffer, NalSize) != AVCDEC_SUCCESS)
        {
            return OMX_FALSE;
        }
    }

    else if (AVC_NALTYPE_SLICE == (AVCNalUnitType) NalType ||
             AVC_NALTYPE_IDR == (AVCNalUnitType) NalType)
    {
        if ((Status = PVAVCDecodeSlice(&(AvcHandle), pNalBuffer, NalSize)) == AVCDEC_PICTURE_OUTPUT_READY)
        {
            FlushOutput_OMX(aOutBuffer, aOutputLength, aOutTimestamp, OldWidth, OldHeight);

            //Input buffer not consumed yet, do not mark it free.
            if (aMarkerFlag)
            {
                *aInBufSize = NalSize;
            }
            else
            {
                *aInBufSize += InputBytesConsumed;
                aInputBuf -= InputBytesConsumed;
            }
        }

        if (Status == AVCDEC_PICTURE_READY)
        {
            (*iFrameCount)++;
        }

        if ((AVCDEC_NO_DATA == Status) || (AVCDEC_PACKET_LOSS == Status) ||
                (AVCDEC_NO_BUFFER == Status) || (AVCDEC_MEMORY_FAIL == Status) ||
                (AVCDEC_FAIL == Status))
        {
            return OMX_FALSE;
        }
    }

    else if ((AVCNalUnitType)NalType == AVC_NALTYPE_SEI)
    {
        if (PVAVCDecSEI(&(AvcHandle), pNalBuffer, NalSize) != AVCDEC_SUCCESS)
        {
            return OMX_FALSE;
        }
    }

    else if ((AVCNalUnitType)NalType == AVC_NALTYPE_AUD)
    {
        //PicType = pNalBuffer[1] >> 5;
    }

    else if ((AVCNalUnitType)NalType == AVC_NALTYPE_EOSTREAM || // end of stream
             (AVCNalUnitType)NalType == AVC_NALTYPE_EOSEQ || // end of sequence
             (AVCNalUnitType)NalType == AVC_NALTYPE_FILL) // filler data
    {
        return OMX_TRUE;
    }

    //else
    //{
    //printf("\nNAL_type = %d, unsupported nal type or not sure what to do for this type\n", NalType);
    //	return OMX_FALSE;
    //}
    return OMX_TRUE;

}


OMX_ERRORTYPE AvcDecoder_OMX::AvcDecDeinit_OMX()
{
    if (pCleanObject)
    {
        OSCL_DELETE(pCleanObject);
        pCleanObject = NULL;
    }

    if (pDpbBuffer)
    {
        oscl_free(pDpbBuffer);
        pDpbBuffer = NULL;
    }

    return OMX_ErrorNone;
}


AVCDec_Status AvcDecoder_OMX::GetNextFullNAL_OMX(uint8** aNalBuffer, int32* aNalSize, OMX_U8* aInputBuf, OMX_U32* aInBufSize)
{
    uint8* pBuff = aInputBuf;
    OMX_U32 InputSize;

    *aNalSize = *aInBufSize;
    InputSize = *aInBufSize;

    AVCDec_Status ret_val = PVAVCAnnexBGetNALUnit(pBuff, aNalBuffer, aNalSize);

    if (ret_val == AVCDEC_FAIL)
    {
        return AVCDEC_FAIL;
    }

    InputBytesConsumed = ((*aNalSize) + (int32)(*aNalBuffer - pBuff));
    aInputBuf += InputBytesConsumed;
    *aInBufSize = InputSize - InputBytesConsumed;

    return AVCDEC_SUCCESS;
}

AVCCleanupObject_OMX::~AVCCleanupObject_OMX()
{
    PVAVCCleanUpDecoder(ipavcHandle);

}

void AvcDecoder_OMX::ResetDecoder()
{
    PVAVCDecReset(&(AvcHandle));
}

