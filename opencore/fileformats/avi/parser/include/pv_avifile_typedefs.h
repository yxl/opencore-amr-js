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
#ifndef PV_AVIFILE_TYPEDEFS_H_INCLUDED
#define PV_AVIFILE_TYPEDEFS_H_INCLUDED

#ifndef OSCLCONFIG_LIMITS_TYPEDEFS_H_INCLUDED
#include "osclconfig_limits_typedefs.h"
#endif

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef OSCL_STRING_UTILS_H_INCLUDED
#include "oscl_string_utils.h"
#endif

#ifndef OSCL_STDSTRING_H_INCLUDED
#include "oscl_stdstring.h"
#endif

#ifndef OSCL_TYPES_H_INCLUDED
#include "oscl_types.h"
#endif

#ifndef OSCL_BYTE_ORDER_H_INCLUDED
#include "oscl_byte_order.h"
#endif

#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

#ifndef OSCL_STRING_CONTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif

#ifndef OSCL_REFCOUNTER_MEMFRAG_H_INCLUDED
#include "oscl_refcounter_memfrag.h"
#endif

#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef PVFILE_H_INCLUDED
#include "pvfile.h"
#endif


#ifndef PV_AVIFILE_STATUS
#include "pv_avifile_status.h"
#endif

//avi file parser specific typedefines
const uint32   CHUNK_SIZE				= 4;
const uint32   RES_BYTES_SZ				= 4;
const uint32   MAX_STRN_SZ				= 100;
const uint32   OFFSET_FROM_MOVI_LST		= 4;
const uint32   MAX_COLOR_TABLE_SIZE		= 256;
const uint32   BIT_COUNT1				= 1;
const uint32   BIT_COUNT2				= 2;
const uint32   BIT_COUNT4				= 4;
const uint32   BIT_COUNT8				= 8;
const uint32   BIT_COUNT16				= 16;
const uint32   BIT_COUNT24				= 24;
const uint32   BIT_COUNT32				= 32;

#define MAKE_FOURCC(a , b, c, d)		((uint32(a) << 24) | (uint32(b) << 16) | (uint32(c) << 8) | uint32(d))

const uint32	RIFF	MAKE_FOURCC('R', 'I', 'F', 'F');
const uint32	AVI		MAKE_FOURCC('A', 'V', 'I', ' ');
const uint32	LIST	MAKE_FOURCC('L', 'I', 'S', 'T');
const uint32	HDRL	MAKE_FOURCC('h', 'd', 'r', 'l');
const uint32	AVIH	MAKE_FOURCC('a', 'v', 'i', 'h');
const uint32	STRL	MAKE_FOURCC('s', 't', 'r', 'l');
const uint32	STRH	MAKE_FOURCC('s', 't', 'r', 'h');
const uint32	STRF	MAKE_FOURCC('s', 't', 'r', 'f');
const uint32	STRN	MAKE_FOURCC('s', 't', 'r', 'n');
const uint32	STRD	MAKE_FOURCC('s', 't', 'r', 'd');
const uint32	IDX1	MAKE_FOURCC('i', 'd', 'x', '1');
const uint32    INDX    MAKE_FOURCC('i', 'n', 'd', 'x');
const uint32	VIDS	MAKE_FOURCC('v', 'i', 'd', 's');
const uint32	AUDS	MAKE_FOURCC('a', 'u', 'd', 's');
const uint32	MIDS	MAKE_FOURCC('m', 'i', 'd', 's');
const uint32	TXTS    MAKE_FOURCC('t', 'x', 't', 's');
const uint32    DIB     MAKE_FOURCC('D', 'I', 'B', ' '); 	//Device-independent Bit Map
const uint32	MOVI    MAKE_FOURCC('m', 'o', 'v', 'i');
const uint32	JUNK	MAKE_FOURCC('J', 'U', 'N', 'K');


#define WAVE_FORMAT_UNKNOWN             (0x0000)

//following are defined by Microsoft
#ifndef WAVE_FORMAT_PCM
#define WAVE_FORMAT_PCM                 (0x0001)
#define WAVE_FORMAT_ADPCM               (0x0002)
#define WAVE_FORMAT_IBM_CVSD            (0x0005)
#define WAVE_FORMAT_ALAW                (0x0006)
#define WAVE_FORMAT_MULAW               (0x0007)
#define WAVE_FORMAT_OKI_ADPCM           (0x0010)
#define WAVE_FORMAT_DVI_ADPCM           (0x0011)
#define WAVE_FORMAT_DIGISTD             (0x0015)
#define WAVE_FORMAT_DIGIFIX             (0x0016)
#define WAVE_FORMAT_YAMAHA_ADPCM        (0x0020)
#define WAVE_FORMAT_DSP_TRUESPEECH      (0x0022)
#define WAVE_FORMAT_GSM610              (0x0031)
#endif
#define IBM_FORMAT_MULAW                (0x0101)
#define IBM_FORMAT_ALAW                 (0x0102)
#define IBM_FORMAT_ADPCM                (0x0103)

#ifndef BI_RGB
#define BI_RGB			  0
#endif
#ifndef BI_REL4
#define BI_REL4			  1
#endif
#ifndef BI_REL8
#define BI_REL8			  2
#endif
#ifndef BI_BITFIELDS
#define BI_BITFIELDS	  3
#endif
#ifndef BI_ALPHABITFIELDS
#define BI_ALPHABITFIELDS 4
#endif

class	PVAviFileParserStatus;
class	PVAviFileParser;
class   PVAviFileHeader;
class   PVAviFileIdxChunk;

typedef enum
{
    PV_2_AUDIO,
    PV_2_VIDEO,
    MIDI,
    TEXT,
    UNKNOWN
}StreamType;


//avih flags
const uint32	AVIF_COPYRIGHTED		= 0x01000000;
const uint32	AVIF_HASINDEX			= 0x10000000;
const uint32	AVIF_ISINTERLEAVED		= 0x100000;
const uint32	AVIF_MUSTUSEINDEX		= 0x010000;
const uint32	AVIF_WASCAPTUREFILE		= 0x100;

//strh flags
const uint32    AVISF_DISABLED          = 0x01000000;
const uint32	AVISF_VIDEO_PALCHANGES  = 0x10000000;

//idx1 flags
const uint32    AVIIF_KEYFRAME          = 0x01000000;
const uint32	AVIIF_LIST				= 0x10000000;
const uint32	AVIIF_NO_TIME           = 0x100000;

typedef struct
{
    uint16 left;
    uint16 top;
    uint16 right;
    uint16 bottom;
}rcFrameType;


//structure to store information present in avi file main header
class PVAviFileMainHeaderStruct
{

    public:
        //constructor
        PVAviFileMainHeaderStruct()
        {
            iMicroSecPerFrame = 0;
            iMaxBytesPerSec = 0;
            iPadding = 0;
            iFlags = 0;
            iIsAVIFileCopyrighted = false;
            iAVIFileHasIndxTbl = false;
            iISAVIFileInterleaved = false;
            iAVIFileMustUseIndex = false;
            iAVIFileWasCaptureFile = false;
            iTotalFrames = 0;
            iInitialFrames = 0;
            iStreams = 0;
            iSuggestedBufferSize = 0;
            iWidth = 0;
            iHeight = 0;

        };

        //destructor
        ~PVAviFileMainHeaderStruct()
        {
            iMicroSecPerFrame = 0;
            iMaxBytesPerSec = 0;
            iPadding = 0;
            iFlags = 0;
            iIsAVIFileCopyrighted = false;
            iAVIFileHasIndxTbl = false;
            iISAVIFileInterleaved = false;
            iAVIFileMustUseIndex = false;
            iAVIFileWasCaptureFile = false;
            iTotalFrames = 0;
            iInitialFrames = 0;
            iStreams = 0;
            iSuggestedBufferSize = 0;
            iWidth = 0;
            iHeight = 0;

        };

        bool	iIsAVIFileCopyrighted;
        bool	iAVIFileHasIndxTbl;
        bool	iISAVIFileInterleaved;
        bool	iAVIFileMustUseIndex;
        bool	iAVIFileWasCaptureFile;
        uint32	iMicroSecPerFrame;
        uint32	iMaxBytesPerSec;
        uint32	iPadding;
        uint32	iFlags;
        uint32	iTotalFrames;
        uint32	iInitialFrames;
        uint32	iStreams;
        uint32	iSuggestedBufferSize;
        uint32	iWidth;
        uint32	iHeight;
        uint32	iReserved[RES_BYTES_SZ];

};

//structure to store stream specific information
class PVAviFileStreamHeaderStruct
{

    public:
        //constructor
        PVAviFileStreamHeaderStruct()
        {
            iStreamTypeFCC = 0;
            iFlags = 0;
            iIsVidPalChangeAvailable = false;
            iAVISF_Disabled = false;
            iPriority = 0;
            iLanguage = 0;
            iInitFrames = 0;
            iScale = 0;
            iRate = 0;
            iStart = 0;
            iLength = 0;
            iSugBufSize = 0;
            iQuality = 0;
            iSampleSize = 0;
            iSamplingRate = 0;

        };

        //destructor
        ~PVAviFileStreamHeaderStruct()
        {
            iStreamTypeFCC = 0;
            iFlags = 0;
            iIsVidPalChangeAvailable = false;
            iAVISF_Disabled = false;
            iPriority = 0;
            iLanguage = 0;
            iInitFrames = 0;
            iScale = 0;
            iRate = 0;
            iStart = 0;
            iLength = 0;
            iSugBufSize = 0;
            iQuality = 0;
            iSampleSize = 0;
            iSamplingRate = 0;
        }


        uint32	iStreamTypeFCC;
        uint32	iFlags;
        bool	iIsVidPalChangeAvailable;
        bool	iAVISF_Disabled;

        //stores codec type info for Audio and Video Stream.
        uint8		iStreamHandler[CHUNK_SIZE];
        uint16		iPriority;
        uint16		iLanguage;
        uint32		iInitFrames;
        uint32		iScale;
        uint32		iRate;
        uint32		iStart;
        uint32		iLength;	//Duration of stream
        uint32		iSugBufSize;
        uint32		iQuality;
        uint32		iSampleSize;
        rcFrameType	ircFrame;
        OsclFloat	iSamplingRate;

};


typedef enum
{
    DIVX,
    CINEPACK,
    INDEO,
    DV,
    MJPEG,
    RGB,
    YUV,
    MP3,
    PCM,
    ADPCM
}StreamHandlerType;

typedef struct
{
    uint32	chunkId;
    uint32	flags;
    uint32	offset;
    uint32	size;
    bool    isKeyFrame;
    bool    ifRecList;
    bool	ifNoTime;

}IdxTblType;

typedef struct
{
    uint8 Red;
    uint8 Green;
    uint8 Blue;
    uint8 Flags;   //alpha entry
}PalletEntryStruct;

typedef struct
{
    uint8  FirstEntry;
    uint8  NumEntries;
    uint32 Flags;
    Oscl_Vector<PalletEntryStruct, OsclMemAllocator> NewEntry;

}AviPalChangeStruct;


typedef struct
{
    uint32	BiSize;
    uint32	BiWidth;
    uint32	BiHeight;
    uint16    BiPlanes;
    uint16    BiBitCount;
    uint32    BiCompression;
    uint32    BiSizeImage;
    uint32    BiXPelsPerMeter;
    uint32    BiYPelsPerMeter;
    uint32    BiClrUsed;
    uint32    BiClrImportant;
}BitmapInfoHhr;


typedef struct
{
    uint8   Blue;
    uint8   Green;
    uint8   Red;
    uint8   Reserved;

}RGBQuad;

typedef struct
{
    BitmapInfoHhr BmiHeader;
    uint32 BmiColorsCount;  //number of RGBQuads actually present
    RGBQuad BmiColors[MAX_COLOR_TABLE_SIZE];

}BitMapInfoStruct;

typedef struct
{
    uint16    FormatTag;
    uint16    Channels;
    uint32    SamplesPerSec;
    uint32    AvgBytesPerSec;
    uint16    BlockAlign;
    uint16    BitsPerSample;
    uint16    Size;

}WaveFormatExStruct;

#endif		//#ifndef PV_AVIFILE_TYPEDEFS_H_INCLUDED

