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
#include "oscl_mem.h"
#include "avcapi_common.h"
#include "avcdec_api.h"

#include "pvavcdecoder.h"


/////////////////////////////////////////////////////////////////////////////
PVAVCDecoder::PVAVCDecoder() : iAVCHandle(NULL)
{
}

PVAVCDecoder* PVAVCDecoder::New(void)
{
    PVAVCDecoder* self = new PVAVCDecoder;

    if (self)
    {
        if (!self->Construct())
        {
            OSCL_DELETE(self);
            self = NULL;
        }
    }

    return self;
}

bool PVAVCDecoder::Construct()
{
    iAVCHandle = (AVCHandle *) new AVCHandle;
    if (iAVCHandle)
    {
        oscl_memset(iAVCHandle, 0, sizeof(iAVCHandle));
        return true;
    }
    else
    {
        return false;
    }
}

/////////////////////////////////////////////////////////////////////////////
PVAVCDecoder::~PVAVCDecoder()
{
    if (iAVCHandle)
    {
        OSCL_DELETE((AVCHandle *)iAVCHandle);
        iAVCHandle = NULL;
    }
}



/////////////////////////////////////////////////////////////////////////////
void PVAVCDecoder::CleanUpAVCDecoder(void)
{
    PVAVCCleanUpDecoder((AVCHandle *)iAVCHandle);
}


void PVAVCDecoder::ResetAVCDecoder(void)
{
    PVAVCDecReset((AVCHandle *)iAVCHandle);
}

////////////////////////////////////////////////////////////////////////////
/* This part is a C-callback function can be in anywhere. */
int CBAVC_Malloc(void *userData, int32 size, int attribute)
{
    PVAVCDecoder *avcDec = (PVAVCDecoder*) userData;

    return avcDec->AVC_Malloc(size, attribute);
}

void CBAVC_Free(void *userData, int mem)
{
    PVAVCDecoder *avcDec = (PVAVCDecoder*) userData;
    avcDec->AVC_Free(mem);
    return ;
}


/////////////////////////////////////////////////////////////////////////////
/* Callback functions for memory allocation/free and request for more data */
/* with C++ wrapper */
int PVAVCDecoder::AVC_Malloc(int32 size, int attribute)
{
    OSCL_UNUSED_ARG(attribute);
    return (int)(oscl_malloc(size));
}

void PVAVCDecoder::AVC_Free(int mem)
{
    oscl_free((uint8*)mem);
}



/////////////////////////////////////////////////////////////////////////////
bool PVAVCDecoder::InitAVCDecoder(FunctionType_SPS init_sps, FunctionType_Alloc alloc_buffer,
                                  FunctionType_Unbind unbind_buffer,
                                  FunctionType_Malloc AVCAlloc,
                                  FunctionType_Free AVCFree,
                                  void *userdata)
{
    /* Initialize videoCtrl */
    ((AVCHandle *)iAVCHandle)->AVCObject = NULL;
    ((AVCHandle *)iAVCHandle)->userData = userdata;

    ((AVCHandle *)iAVCHandle)->CBAVC_DPBAlloc = init_sps;
    ((AVCHandle *)iAVCHandle)->CBAVC_FrameBind = alloc_buffer;
    ((AVCHandle *)iAVCHandle)->CBAVC_FrameUnbind = unbind_buffer;
    ((AVCHandle *)iAVCHandle)->CBAVC_Malloc = AVCAlloc;
    ((AVCHandle *)iAVCHandle)->CBAVC_Free = AVCFree;

    return true;
}
/////////////////////////////////////////////////////////////////////////////

int32 PVAVCDecoder::DecodeSPS(uint8 *bitstream, int32 buffer_size)
{
    return PVAVCDecSeqParamSet((AVCHandle *)iAVCHandle, bitstream, buffer_size);
}

int32 PVAVCDecoder::DecodePPS(uint8 *bitstream, int32 buffer_size)
{
    return PVAVCDecPicParamSet((AVCHandle *)iAVCHandle, bitstream, buffer_size);
}

int32 PVAVCDecoder::DecodeAVCSlice(uint8 *bitstream, int32 *buffer_size)
{
    return (PVAVCDecodeSlice((AVCHandle *)iAVCHandle, bitstream, *buffer_size));
}

bool PVAVCDecoder::GetDecOutput(int *indx, int *release)
{
    AVCFrameIO output;
    return (PVAVCDecGetOutput((AVCHandle *)iAVCHandle, indx, release, &output) != AVCDEC_SUCCESS) ? false : true;
}


void PVAVCDecoder::GetVideoDimensions(int32 *width, int32 *height, int32 *top, int32 *left, int32 *bottom, int32 *right)
{
    AVCDecSPSInfo seqInfo;
    PVAVCDecGetSeqInfo((AVCHandle *)iAVCHandle, &seqInfo);
    *width = seqInfo.FrameWidth;
    *height = seqInfo.FrameHeight;

    /* assuming top left corner aligned */
    *top = seqInfo.frame_crop_top;
    *left = seqInfo.frame_crop_left;
    *bottom = seqInfo.frame_crop_bottom;
    *right = seqInfo.frame_crop_right;
}
