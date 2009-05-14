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
/*
    This PVA_FF_MediaDataAtom Class contains the media data.  This class can operate in
    either one of two ways - 1. it can store all it's data in memory (such as
    during the creation of PVA_FF_ObjectDescriptor streams), or 2. it can maintain all
    it's data on disk (such as during the creation ofmedia streams - i.e. audio
    and video).

    Note that during reading in this atom from a file stream, the type fp forced
    to MEDIA_DATA_ON_DISK thereby keeping all the object data in the physical
    file.
*/

#define IMPLEMENT_MediaDataAtom

#include "mediadataatom.h"
#include "atomutils.h"
#include "a_atomdefs.h"
#include "oscl_byte_order.h"
#include "oscl_bin_stream.h"

#define TEMP_TO_TARGET_FILE_COPY_BLOCK_SIZE 1024

typedef Oscl_Vector<PVA_FF_Renderable*, OsclMemAllocator> PVA_FF_RenderableVecType;
typedef Oscl_Vector<PVA_FF_TrackAtom*, OsclMemAllocator> PVA_FF_TrackAtomVecType;

// Constructor
PVA_FF_MediaDataAtom::PVA_FF_MediaDataAtom(PVA_FF_UNICODE_STRING_PARAM outputPathString,
        PVA_FF_UNICODE_STRING_PARAM postfixString,
        int32 tempFileIndex,
        int32 type,
        void  *osclFileServerSession,
        uint32 aCacheSize)
        : PVA_FF_Atom(MEDIA_DATA_ATOM)
{
    _osclFileServerSession = osclFileServerSession;

    _success = true;
    _prenderables = NULL;
    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_RenderableVecType, (), _prenderables);
    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_TrackAtomVecType, (), _ptrackReferencePtrVec);

    // ADDED TO CHECK FOR ANY FILE WRITE FAILURES
    _fileWriteError = false;
    _targetFileWriteError = false;
    _directRender = false;
    _oIsFileOpen = false;

    _fileSize = 0;
    _fileOffsetForChunkStart = 0;
    _fileOffsetForAtomStart = 0;
    _type = type;

    _pofstream._filePtr = NULL;
    _ptrackReferencePtr = NULL;
    _targetFileMediaStartOffset = 0;
    _totalDataRenderedToTargetFile = 0;

    _tempFilePostfix = postfixString;

    _tempFilename = outputPathString;

    _tempFileIndex = tempFileIndex;

    recomputeSize();

    // Preparing the temp output file for rendering the atom data
    if (_type == MEDIA_DATA_ON_DISK)
    {
        prepareTempFile(aCacheSize);
    }
}

PVA_FF_MediaDataAtom::PVA_FF_MediaDataAtom(PVA_FF_UNICODE_STRING_PARAM targetFileName,
        void  *osclFileServerSession, uint32 aCacheSize)
        : PVA_FF_Atom(MEDIA_DATA_ATOM)
{
    _type = MEDIA_DATA_ON_DISK;

    _osclFileServerSession = osclFileServerSession;
    _targetFileMediaStartOffset = 0;
    _totalDataRenderedToTargetFile = 0;
    _prenderables = NULL;
    _success = true;
    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_RenderableVecType, (), _prenderables);
    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_TrackAtomVecType, (), _ptrackReferencePtrVec);


    // ADDED TO CHECK FOR ANY FILE WRITE FAILURES
    _fileWriteError = false;
    _targetFileWriteError = false;
    _fileSize = 0;
    _fileOffsetForChunkStart = 0;
    _fileOffsetForAtomStart = 0;

    _directRender = true;

    _ptrackReferencePtr = NULL;

    recomputeSize();

    _pofstream._filePtr = NULL;
    _pofstream._osclFileServerSession = OSCL_STATIC_CAST(Oscl_FileServer*, _osclFileServerSession);


    int retVal = PVA_FF_AtomUtils::openFile(&_pofstream, targetFileName, Oscl_File::MODE_READWRITE | Oscl_File::MODE_BINARY, aCacheSize);
    _oIsFileOpen = true;

    if (_pofstream._filePtr == NULL)
    {
        _fileWriteError = true;
    }
    else if (retVal == 0)
    {
        _targetFileWriteError = true;
        if (_pofstream._filePtr != NULL)
        {
            PVA_FF_AtomUtils::closeFile(&_pofstream);
            _pofstream._filePtr = NULL;
        }
    }
}

PVA_FF_MediaDataAtom::PVA_FF_MediaDataAtom(MP4_AUTHOR_FF_FILE_HANDLE targetFileHandle,
        void  *osclFileServerSession, uint32 aCacheSize)
        : PVA_FF_Atom(MEDIA_DATA_ATOM)
{
    OSCL_UNUSED_ARG(aCacheSize);
    _type = MEDIA_DATA_ON_DISK;

    _osclFileServerSession = osclFileServerSession;
    _targetFileMediaStartOffset = 0;
    _totalDataRenderedToTargetFile = 0;
    _prenderables = NULL;

    _success = true;
    // _ptrackReferencePtrVec = new Oscl_Vector<PVA_FF_TrackAtom*,OsclMemAllocator>();
    // _prenderables = new Oscl_Vector<PVA_FF_Renderable*,OsclMemAllocator>();

    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_RenderableVecType, (), _prenderables);
    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_TrackAtomVecType, (), _ptrackReferencePtrVec);



    // ADDED TO CHECK FOR ANY FILE WRITE FAILURES
    _fileWriteError = false;
    _targetFileWriteError = false;

    _fileSize = 0;
    _fileOffsetForChunkStart = 0;
    _fileOffsetForAtomStart = 0;
    _oIsFileOpen = false;
    _directRender = true;

    _ptrackReferencePtr = NULL;

    recomputeSize();

    _pofstream._filePtr = targetFileHandle;

    if (_pofstream._filePtr == NULL)
    {
        _fileWriteError = true;
    }
}

// Destructor
PVA_FF_MediaDataAtom::~PVA_FF_MediaDataAtom()
{
    if (_pofstream._filePtr != NULL && true == _oIsFileOpen)
    {
        PVA_FF_AtomUtils::closeFile(&_pofstream);
        _pofstream._filePtr = NULL;
    }

    // PVA_FF_TrackAtom *_ptrackReferencePtr - is taken care of by the movie atom
    // Delete vector<PVA_FF_Renderable*> *_prenderables
    if (_prenderables != NULL)
    {
        for (uint32 i = 0; i < _prenderables->size(); i++)
        {
            if ((*_prenderables)[i] != NULL)
            {
                OSCL_DELETE((*_prenderables)[i]);
                //PV_MP4_FF_DELETE(NULL,PVA_FF_Renderable,(*_prenderables)[i]);
                (*_prenderables)[i] = NULL;
            }
        }
        PV_MP4_FF_TEMPLATED_DELETE(NULL, PVA_FF_RenderableVecType, Oscl_Vector, _prenderables);
        _prenderables = NULL;
    }

    //Contents of this array are deleted in movie atom
    //OSCL_DELETE(_ptrackReferencePtrVec);

    PV_MP4_FF_TEMPLATED_DELETE(NULL, PVA_FF_TrackAtomVecType, Oscl_Vector, _ptrackReferencePtrVec);

    Oscl_FileServer fileServ;
    fileServ.Connect();
    fileServ.Oscl_DeleteFile(_tempFilename.get_cstr());
    fileServ.Close();
}

// Create the atom temp file and the corresponding ofstream
void
PVA_FF_MediaDataAtom::prepareTempFile(uint32 aCacheSize)
{
    if (_pofstream._filePtr == NULL && !_fileWriteError)
    {
        // 05/31/01 Generate temporary files into output path (the actual mp4 location)
        // _tempFilename already contains the output path ("drive:\\...\\...\\")
        //
        _tempFilename += _STRLIT("temp");
        // Assign the rest of the temp filename - index plus suffix
        _tempFilename += (uint16)(_tempFileIndex++);

        // 03/21/01 Multiple instances support
        _tempFilename += _STRLIT("_");
        _tempFilename += _tempFilePostfix;
        //

        _tempFilename += _STRLIT(".mdat");

        _pofstream._osclFileServerSession = OSCL_STATIC_CAST(Oscl_FileServer*, _osclFileServerSession);

        PVA_FF_AtomUtils::openFile(&_pofstream, _tempFilename, Oscl_File::MODE_READWRITE | Oscl_File::MODE_BINARY, aCacheSize);

        if (_pofstream._filePtr == NULL)
        {
            _fileWriteError = true;
        }
        else
        {
            _oIsFileOpen = true;
        }

        // Render the atoms base members to the media data atom file
        renderAtomBaseMembers(&_pofstream);

        _fileOffsetForChunkStart = getDefaultSize();
        _fileSize = getDefaultSize();
    }
}

bool
PVA_FF_MediaDataAtom::prepareTargetFile(uint32 mediaStartOffset)
{
    if (_directRender)
    {
        if ((_pofstream._filePtr != NULL) && (_fileWriteError != true))
        {
            if (mediaStartOffset > 0)
            {
                // Write zeros to accomodate the user data upfront
                uint8* tempBuffer = NULL;
                PV_MP4_FF_ARRAY_NEW(NULL, uint8, mediaStartOffset, tempBuffer);

                oscl_memset(tempBuffer, 0, mediaStartOffset);

                if (!(PVA_FF_AtomUtils::renderByteData(&_pofstream, mediaStartOffset, tempBuffer)))
                {
                    PV_MP4_ARRAY_DELETE(NULL, tempBuffer);
                    return false;
                }
                PV_MP4_ARRAY_DELETE(NULL, tempBuffer);
            }

            // Render the atoms base members to the media data atom file
            renderAtomBaseMembers(&_pofstream);

            _fileOffsetForChunkStart = getDefaultSize();
            _fileSize = getDefaultSize();

            _targetFileMediaStartOffset = mediaStartOffset;

            return true;
        }
    }
    return false;
}


uint32
PVA_FF_MediaDataAtom::prepareTargetFileForFragments(uint32 mediaStartOffset)
{
    if (_directRender)
    {
        _targetFileMediaStartOffset = mediaStartOffset;
        PVA_FF_AtomUtils::seekFromStart(&_pofstream, _targetFileMediaStartOffset);

        renderAtomBaseMembers(&_pofstream);
        _fileOffsetForChunkStart = getDefaultSize();

        _fileSize = getDefaultSize();

        return _fileOffsetForChunkStart;
    }

    return 0;
}

bool
PVA_FF_MediaDataAtom::closeTargetFile()
{
    if (_directRender)
    {
        if ((_pofstream._filePtr != NULL) && (_fileWriteError != true))
        {
            // Get current position of put pointer
            _totalDataRenderedToTargetFile =
                PVA_FF_AtomUtils::getCurrentFilePosition(&_pofstream);

            // Go to the beginning of the media data
            PVA_FF_AtomUtils::seekFromStart(&_pofstream, _targetFileMediaStartOffset);

            // Update size field
            if (!PVA_FF_AtomUtils::render32(&_pofstream, getSize()))
            {
                return false;
            }

            // Return the _pofstream's pointer to start
            PVA_FF_AtomUtils::seekFromStart(&_pofstream, 0);

            _fileOffsetForChunkStart =
                _targetFileMediaStartOffset + getDefaultSize();

            return true;
        }
    }
    return false;
}

Oscl_File*
PVA_FF_MediaDataAtom::getTargetFilePtr()
{
    return (_pofstream._filePtr);
}

// Adds more data to the atom then update the atom size field (first 4 bytes)
bool
PVA_FF_MediaDataAtom::addRawSample(void *psample, uint32 length)
{
    bool retVal = true;

    if (_type == MEDIA_DATA_ON_DISK)
    {
        if (!_fileWriteError)
        {
            if (_pofstream._filePtr == NULL)
            {
                if (!_directRender)
                {
                    // If initial file fp not opened
                    prepareTempFile();
                }
                else
                {
                    //File must have been prepared for direct render
                    return false;
                }
            }

            bool ret = PVA_FF_AtomUtils::renderByteData(&_pofstream, length, (uint8 *)psample);

            if (ret == false)
            {
                _fileWriteError = true;
                retVal = false;
            }

            _fileSize += length; // Update the size of the atom

            // Update the size of the atom
            recomputeSize();
        }
        else
        {
            retVal = false;
        }
    }
    else
    {
        retVal = false;
    }

    return (retVal);
}

bool PVA_FF_MediaDataAtom::addRawSample(Oscl_Vector <OsclMemoryFragment, OsclMemAllocator>& fragmentList,
                                        uint32 length, int32 mediaType, int32 codecType)
{
    bool retVal = true;
    bool ret = true;
    uint32 ii = 0;
    OsclBinIStreamBigEndian stream;

    if (_type == MEDIA_DATA_ON_DISK)
    {
        if (!_fileWriteError)
        {
            if (_pofstream._filePtr == NULL)
            {
                if (!_directRender)
                {
                    // If initial file fp not opened
                    prepareTempFile();
                }
                else
                {
                    //File must have been prepared for direct render
                    return false;
                }
            }

            uint32 nalLength = 0;
            if (mediaType == (int32)MEDIA_TYPE_VISUAL && codecType == CODEC_TYPE_AVC_VIDEO)
            {
                for (ii = 0; ii < fragmentList.size(); ii++)
                {
                    // read NAL length in Big Endian format
                    stream.Attach((OsclAny*) &(fragmentList[ii].len), 4);
                    stream >> nalLength;

                    // compose nal length in two bytes
                    ret = PVA_FF_AtomUtils::renderByteData(&_pofstream, 4, (uint8 *) & nalLength);
                    if (ret == false)
                    {
                        _fileWriteError = true;
                        retVal = false;
                    }

                    // write NAL uint
                    ret = PVA_FF_AtomUtils::renderByteData(&_pofstream, fragmentList[ii].len, (uint8 *)fragmentList[ii].ptr);
                    if (ret == false)
                    {
                        _fileWriteError = true;
                        retVal = false;
                    }
                }
            }
            else
            {
                for (ii = 0; ii < fragmentList.size(); ii++)
                {
                    ret = PVA_FF_AtomUtils::renderByteData(&_pofstream, fragmentList[ii].len, (uint8 *)fragmentList[ii].ptr);
                }

            }

            if (ret == false)
            {
                _fileWriteError = true;
                retVal = false;
            }

            _fileSize += length; // Update the size of the atom

            // Update the size of the atom
            recomputeSize();
        }
        else
        {
            retVal = false;
        }
    }
    else
    {
        retVal = false;
    }

    return (retVal);
}

int32
PVA_FF_MediaDataAtom::addRenderableSample(PVA_FF_Renderable *psample)
{
    if (_type == MEDIA_DATA_ON_DISK)
    {
        // Force renderables to
        // be written to disk
        uint32 length = psample->getSize();
        psample->renderToFileStream(&_pofstream);
        _fileSize += length;

        recomputeSize();
        return length;
    }
    else
    {
        // MEDIA_DATA_IN_MEMORY
        PVA_FF_Renderable *prenderable = (PVA_FF_Renderable*) psample;
        _prenderables->push_back(prenderable);

        recomputeSize();
        return prenderable->getSize();
    }
}


// Allocates in-memory space for the media data
void
PVA_FF_MediaDataAtom::reserveBuffer(int32 size)
{
    OSCL_UNUSED_ARG(size);
}

void
PVA_FF_MediaDataAtom::recomputeSize()
{
    if (_type == MEDIA_DATA_ON_DISK)
    {
        // Entire atom size fp same as atom file size
        if (_fileSize == 0)
        {
            _size = getDefaultSize();
        }
        else
        {
            _size = _fileSize;
        }
    }
    else
    { // MEDIA_DATA_IN_MEMORY

        uint32 size = getDefaultSize();

        // Include size from actual data payload

        // From renderable data
        for (uint32 i = 0; i < _prenderables->size(); i++)
        {
            size += (*_prenderables)[i]->getSize();
        }

        _size = size;
    }
}

uint32
PVA_FF_MediaDataAtom::getMediaDataSize()
{
    recomputeSize();

    uint32 size = getSize();

    return (size);
}

// Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
bool
PVA_FF_MediaDataAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{

    int32 rendered = 0; // Keep track of number of bytes rendered

    // Render the data
    if (_type == MEDIA_DATA_IN_MEMORY)
    {

        // Render in-memory data directoy to disk
        // From renderable data

        // Render PVA_FF_Atom type and size
        if (!renderAtomBaseMembers(fp))
        {
            return false;
        }
        rendered += getDefaultSize();

        // BREAKING CONST RULES!!!

        // Need to set the actual file offset where the actual chunk data begins
        // so before rendering the ChunkOffetAtom, we can shift the PVA_FF_ChunkOffsetAtom
        // table elements by this offset - the table elements are actual file offsets
        // and NOT just offsets from the first chunk (i.e. zero) and we don't really
        // know this offset until now.
        PVA_FF_MediaDataAtom *This = const_cast<PVA_FF_MediaDataAtom*>(this);
        This->setFileOffsetForChunkStart(PVA_FF_AtomUtils::getCurrentFilePosition(fp));

        for (uint32 i = 0; i < _prenderables->size(); i++)
        {
            if (!(*_prenderables)[i]->renderToFileStream(fp))
            {
                return false;
            }
            rendered += (*_prenderables)[i]->getSize();
        }
    }
    else
    {
        // MEDIA_DATA_ON_DISK
        // 05/30/01 CPU problem when the file fp big.
        // We update the size at the end not for every sample.
        // Need to update the atoms size field on disk
        int32 currentPos = PVA_FF_AtomUtils::getCurrentFilePosition(&_pofstream);    // Get current position of put pointer
        PVA_FF_AtomUtils::seekFromStart(&_pofstream, 0);          // Go to the beginning of the file
        if (!PVA_FF_AtomUtils::render32(&_pofstream, getSize()))
        {
            return false;
        }
        // Update size field
        PVA_FF_AtomUtils::seekFromStart(&_pofstream, currentPos); // Return the ostream's put pointer

        // Cleanup and close temp data output file
        if (_pofstream._filePtr != NULL)
        {
            PVA_FF_AtomUtils::closeFile(&_pofstream);
            _pofstream._filePtr = NULL;
        }

        // Open the file in which this mdat atom was stored
        MP4_AUTHOR_FF_FILE_IO_WRAP mdatFilePtr;
        mdatFilePtr._filePtr = NULL;
        mdatFilePtr._osclFileServerSession = OSCL_STATIC_CAST(Oscl_FileServer*, _osclFileServerSession);
        PVA_FF_AtomUtils::openFile(&mdatFilePtr, _tempFilename, Oscl_File::MODE_READ | Oscl_File::MODE_BINARY);

        // Seek to the offset in the file where the ATOM starts
        PVA_FF_AtomUtils::seekFromStart(&mdatFilePtr, _fileOffsetForAtomStart);

        // In the case where the mdat atom fp stored on disk file,
        // the atom just gets directly copied - i.e. there fp no atom-specific
        // rendering.  We need to adjust the fileOffset by the size of the
        // atom header (based on what the header "should" be).

        // BREAKING CONST RULES!!!

        // Need to set the actual file offset where the actual chunk data begins
        // so before rendering the ChunkOffetAtom, we can shift the PVA_FF_ChunkOffsetAtom
        // table elements by this offset - the table elements are actual file offsets
        // and NOT just offsets from the first chunk (i.e. zero) and we don't really
        // know this offset until now (during rendering).
        PVA_FF_MediaDataAtom *This = const_cast<PVA_FF_MediaDataAtom*>(this);
        This->setFileOffsetForChunkStart((uint32)(PVA_FF_AtomUtils::getCurrentFilePosition(fp)) +
                                         (uint32)getDefaultSize());

        // Read in atom from separate file and copy byte-by-byte to new ofstream
        // (including the mediaDataAtom header - 4 byte size ad 4 byte type)

        uint32 readBlockSize = 0;
        uint32 tempFileSize  = getSize();

        uint8 *dataBuf = NULL;

        PV_MP4_FF_ARRAY_NEW(NULL, uint8, TEMP_TO_TARGET_FILE_COPY_BLOCK_SIZE, dataBuf);

        while (tempFileSize > 0)
        {
            if (tempFileSize < TEMP_TO_TARGET_FILE_COPY_BLOCK_SIZE)
            {
                readBlockSize = tempFileSize;
            }
            else
            {
                readBlockSize = TEMP_TO_TARGET_FILE_COPY_BLOCK_SIZE;
            }

            if (!(PVA_FF_AtomUtils::readByteData(&mdatFilePtr, readBlockSize, dataBuf)))
            {
                _targetFileWriteError = true;
                return false;
            }

            if (!(PVA_FF_AtomUtils::renderByteData(fp, readBlockSize, dataBuf)))
            {
                _targetFileWriteError = true;
                return false;
            }
            tempFileSize -= readBlockSize;
        }

        PV_MP4_FF_DELETE(NULL, uint8, dataBuf);

        rendered += _fileSize;

        PVA_FF_AtomUtils::closeFile(&mdatFilePtr);

    }

    return true;
}

