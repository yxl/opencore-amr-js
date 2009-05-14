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
/* //////////////////////////////////////////////////////////////// */
/*	File	: avi_write.h											*/
/* ---------------------------------------------------------------- */
/*	Create AVI file from a YUV or RGB output of MPEG4 decoder.		*/
/* //////////////////////////////////////////////////////////////// */

#ifndef AVI_WRITE_H_INCLUDED
#define AVI_WRITE_H_INCLUDED

#include "oscl_string.h"
#include "oscl_file_io.h"

#ifndef mmioFOURCC
#define mmioFOURCC( ch0, ch1, ch2, ch3 ) \
    ( (int)(unsigned char)(ch0) | ( (int)(unsigned char)(ch1) << 8 ) | \
    ( (int)(unsigned char)(ch2) << 16 ) | ( (int)(unsigned char)(ch3) << 24 ) )
#endif

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#ifndef BI_RGB
#define BI_RGB        0L
#endif

#define streamtypeVIDEO		mmioFOURCC('v', 'i', 'd', 's')
#define formtypeAVI         mmioFOURCC('A', 'V', 'I', ' ')
#define FOURCC_LIST			mmioFOURCC('L', 'I', 'S', 'T')
#define FOURCC_RIFF			mmioFOURCC('R', 'I', 'F', 'F')
#define FOURCC_WAVE			mmioFOURCC('W', 'A', 'V', 'E')
#define FOURCC_fmt			mmioFOURCC('f', 'm', 't', ' ')
#define FOURCC_data			mmioFOURCC('d', 'a', 't', 'a')
#define listtypeAVIHEADER       mmioFOURCC('h', 'd', 'r', 'l')
#define listtypeSTREAMHEADER    mmioFOURCC('s', 't', 'r', 'l')
#define ckidSTREAMHEADER        mmioFOURCC('s', 't', 'r', 'h')
#define ckidSTREAMFORMAT        mmioFOURCC('s', 't', 'r', 'f')
#define ckidAVIMAINHDR          mmioFOURCC('a', 'v', 'i', 'h')
#define ckidAVIPADDING		mmioFOURCC('J', 'U', 'N', 'K')
#define listtypeAVIMOVIE	mmioFOURCC('m', 'o', 'v', 'i')
#define videoChunkID        mmioFOURCC('0', '0', 'd', 'b')
#define ckidAVINEWINDEX		mmioFOURCC('i', 'd', 'x', '1')

#define AVIF_HASINDEX_FILE_OUT		0x00000010	// Index at end of file?
#define AVIF_TRUSTCKTYPE_FILE_OUT	0x00000800	// Use CKType to find key frames?


#define DEFAULT_COUNT 20000 /* default number of frames */

typedef struct
{
    int16	left;
    int16	top;
    int16	right;
    int16	bottom;
} rect;


typedef struct
{
    uint32	dwMicroSecPerFrame;	// frame display rate (or 0L)
    uint32	dwMaxBytesPerSec;	// max. transfer rate
    uint32	dwPaddingGranularity;	// pad to multiples of this
    // size; normally 2K.
    uint32	dwFlags;		// the ever-present flags
    uint32	dwTotalFrames;		// # frames in file
    uint32	dwInitialFrames;
    uint32	dwStreams;
    uint32	dwSuggestedBufferSize;

    uint32	dwWidth;
    uint32	dwHeight;

    uint32	dwScale;
    uint32	dwRate;
    uint32	dwStart;
    uint32	dwLength;
} AVIMainHeader;

/*
 * Stream header
 */

//#define AVISF_DISABLED			0x00000001
//#define AVISF_VIDEO_PALCHANGES		0x00010000

typedef struct
{
    uint32	fccType;
    uint32	fccHandler;
    uint32	dwFlags;	/* Contains AVISF_* flags */
    uint16	wPriority;	/* dwPriority - splited for audio */
    uint16	wLanguage;
    uint32	dwInitialFrames;
    uint32	dwScale;
    uint32	dwRate;		/* dwRate / dwScale == samples/second */
    uint32	dwStart;
    uint32	dwLength;	/* In units above... */
    uint32	dwSuggestedBufferSize;
    int32	dwQuality;
    uint32	dwSampleSize;
    rect	rcFrame;
} AVIStreamHeader;

typedef struct
{
    uint32	chunkID;
    uint32	flags;
    uint32	offset;
    uint32	length;
} AVIIndex;

typedef struct
{
    uint32	length;
    uint8   indexBuffer[20000 * 16 + 2];
} IndexBuffer;


typedef struct
{
    uint32      biSize;
    int32       biWidth;
    int32       biHeight;
    uint16       biPlanes;
    uint16       biBitCount;
    uint32      biCompression;
    uint32      biSizeImage;
    int32       biXPelsPerMeter;
    int32       biYPelsPerMeter;
    uint32      biClrUsed;
    uint32      biClrImportant;
} BitMapInfoHeader;

void InitializeAVI(OSCL_TCHAR *filename, int width, int height, int cc);

void WriteSampleAVI(unsigned char *src, long timestamp);

void CleanUpAVI(long lasttime);

#endif
