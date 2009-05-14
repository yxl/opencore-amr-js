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
* =============================================================================
*  Name        : oscl_file_async_read.cpp
*  Part of     :
*  Interface   :
*  Description :
*  Version     :
* =============================================================================
*/



#include "oscl_file_async_read.h"
#include "oscl_file_handle.h"
#include "pvlogger.h"
#include "oscl_file_native.h"

//When set, this code will create a duplicate file handle and
//verify every single API call against results from that handle.
//Great for debugging.
//Make sure this is 0 in checked-in code!
#define VERIFY_THIS 0

// ==================   Class OsclAsyncFileBuffer =====================================  //

OsclAsyncFileBuffer* OsclAsyncFileBuffer::NewL(int32 aBufferSize, int32 aId)
{
    OsclAsyncFileBuffer* self = OSCL_NEW(OsclAsyncFileBuffer, (aBufferSize, aId));
    OsclError::PushL(self);
    self->ConstructL();
    OsclError::Pop();
    return self;
}

OsclAsyncFileBuffer::OsclAsyncFileBuffer(int32 aBufferSize, int32 aId) :
        iBufferSize(aBufferSize), iId(aId)
{
    iBuffer = NULL;
    iOffset = 0;
    iInUse = false;
    iLength = 0;
    iValid = false;
}

OsclAsyncFileBuffer::~OsclAsyncFileBuffer()
{
    if (iBuffer)
        OsclBuf::Delete(iBuffer);
}

void OsclAsyncFileBuffer::ConstructL()
{
    iBuffer = OsclBuf::NewL(iBufferSize);
}

void OsclAsyncFileBuffer::UpdateData()
{
    iValid = true;
    iLength = iBuffer->Length();
}

OsclBuf* OsclAsyncFileBuffer::Buffer()
{
    return iBuffer;
}

bool OsclAsyncFileBuffer::HasThisOffset(TOsclFileOffset aOffset)
{
    if (!iValid)
        return false;
    if ((aOffset >= iOffset) && (aOffset <= iOffset + iLength - 1))
        return true;
    return false;
}


// ==================   Class OsclAsyncFile =====================================  //

OsclAsyncFile* OsclAsyncFile::NewL(OsclNativeFile& aFile, int32 aCacheSize, PVLogger* aLogger)
{
    OsclAsyncFile* self = OSCL_NEW(OsclAsyncFile, (aFile, aCacheSize, aLogger));
    OsclError::PushL(self);
    self->ConstructL();
    OsclError::Pop();
    return self;
}

void OsclAsyncFile::Delete(OsclAsyncFile*a)
{
    if (a)
        OSCL_DELETE(a);
}

OsclAsyncFile::~OsclAsyncFile()
{
    //Stop the tread, cancel any requests for Run()
    //and remove AO from the scheduler

    StopAsyncReadThread();

    Cancel();
    RemoveFromScheduler();

    if (iNativeFileDuplicate)
    {
        OSCL_DELETE(iNativeFileDuplicate);
    }

    for (uint32 i = 0; i < iDataBufferArray.size(); i++)
        OSCL_DELETE(iDataBufferArray[i]);

    iDataBufferArray.clear();
    iSortedDataBufferArray.clear();
    iLinkedDataBufferArray.clear();
    OSCL_DELETE(iDataBuffer);

}


OsclAsyncFile::OsclAsyncFile(OsclNativeFile& aFile, int32 aCacheSize, PVLogger* aLogger)
        : OsclActiveObject(OsclActiveObject::EPriorityHighest, "OsclAsyncFile"),
        iNativeFile(aFile),
        iNativeFileDuplicate(NULL),
        iTotalCacheSize(aCacheSize),
        iStartAsyncRead(false),
        iReadPtrDummyLen(0),
        iReadPtr(0, iReadPtrDummyLen, 0)
{
    iLogger = aLogger;

    iNumOfRun = 0;
    iNumOfRunErr = 0;
    iHasNativeAsyncRead = iNativeFile.HasAsyncRead();

    // Init thread state tracking variable(s)
    if (iHasNativeAsyncRead)
    {
        /* For native async read set the thread state to active
         * since this is logically equivalent to the read thread
         * already running for the non-native implementation.  Some
         * of the shared logic in this class uses this state variable
         * so it needs to be set properly for the native async case also.
         */
        iAsyncReadThreadState = EAsyncReadActive;
    }
    else
    {
        /* otherwise there is not native async read support so this
         * class will need to launch a reader thread.  Initialize the
         * state to EAsyncReadNotActive to record the fact that the thread
         * has not been launched yet.
         */
        iAsyncReadThreadState = EAsyncReadNotActive;
    }
    iAsyncReadThreadExitFlag = false;



    //this is the number of buffers we allocate internally for the linked buffer
    //list.  There is also one extra buffer of this size-- the 'iDataBuffer'.
    iKCacheBufferCount = 4;

    //'min bytes read ahead' is how much data we will try to keep available
    //from the async reads.  It must be <= iKCacheBufferCount * iTotalCacheSize;
    iKMinBytesReadAhead = (3 * iTotalCacheSize);
    OSCL_ASSERT(iKMinBytesReadAhead <= (int32)(iKCacheBufferCount*iTotalCacheSize));

    //this is the size of each read request to the native file system.
    //it must be <= the individual buffer size, so we use 8k as the default,
    //but then limit it to the input buffer size.
    iKAsyncReadBufferSize = 8 * 1024 ;
    if ((uint32)iKAsyncReadBufferSize > iTotalCacheSize)
        iKAsyncReadBufferSize = iTotalCacheSize;

}

void OsclAsyncFile::ConstructL()
{
    //Create a duplicate file handle
    //@todo: on symbian we need to make sure this is a duplicate of iNativeFile.
    //on non-symbian it doesn't matter-- you can just create a new instance.
    iNativeFileDuplicate = OSCL_NEW(OsclNativeFile, ());

#if(VERIFY_THIS)
    iNativeFileVerify = OSCL_NEW(OsclNativeFile, ());
    iVerifyCount = 0;
#endif

    iFileSize = iNativeFile.Size();

    iDataBufferArray.reserve(iKCacheBufferCount);

    // Create data buffers

    OsclAsyncFileBuffer* tmpBuffer;
    for (int32 i = 0; i < iKCacheBufferCount; i++)
    {
        tmpBuffer = OsclAsyncFileBuffer::NewL(iTotalCacheSize, i);
        iDataBufferArray.push_back(tmpBuffer);
    }

    // create a local instance of OsclAsyncFileBuffer
    iDataBuffer = OsclAsyncFileBuffer::NewL(iTotalCacheSize, -1);

    AddToScheduler();

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                    (0, "OsclAsyncFile(0x%x)::ConstructL bufferSize %d numBuffers %d readSize %d readAhead %d"
                     , this, iTotalCacheSize, iKCacheBufferCount, iKAsyncReadBufferSize, iKMinBytesReadAhead));
}

int32  OsclAsyncFile::Open(const oscl_wchar *filename, uint32 mode
                           , const OsclNativeFileParams& params
                           , Oscl_FileServer& fileserv)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                    (0, "OsclAsyncFile(0x%x)::Open mode %d ", this, mode));

    //open the duplicate file handle
    if (!iNativeFileDuplicate)
        return -1;
    int32 result = iNativeFileDuplicate->Open(filename, mode, params, fileserv);

    //launch the thread
    if (!iHasNativeAsyncRead)
    {
        LaunchAsyncReadThread();
    }

    iFilePosition = 0;
    iSyncFilePosition = 0;
    iAsyncFilePosition = 0;
    iLastUserFileRead = 0;

    //start the async read if file is opened and thread is running
    if ((result == 0) && (iAsyncReadThreadState == EAsyncReadActive))
    {
        StartAsyncRead(true);
    }
    else
    {
        result = -1;
    }

#if(VERIFY_THIS)
    if (iNativeFileVerify->Open(filename, mode, params, fileserv) != result)
        OSCL_ASSERT(0);
#endif
    return result;
}

int32  OsclAsyncFile::Open(const char *filename, uint32 mode
                           , const OsclNativeFileParams& params
                           , Oscl_FileServer& fileserv)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                    (0, "OsclAsyncFile(0x%x)::Open filename '%s' mode %d ", this, filename, mode));

    //open the duplicate file handle
    if (!iNativeFileDuplicate)
        return -1;
    int32 result = iNativeFileDuplicate->Open(filename, mode, params, fileserv);

    //launch the thread
    if (!iHasNativeAsyncRead)
    {
        LaunchAsyncReadThread();
    }

    iFilePosition = 0;
    iSyncFilePosition = 0;
    iAsyncFilePosition = 0;
    iLastUserFileRead = 0;

    //start the async read if file is opened and thread is running
    if ((result == 0) && (iAsyncReadThreadState == EAsyncReadActive))
    {
        StartAsyncRead(true);
    }
    else
    {
        result = -1;
    }

#if(VERIFY_THIS)
    if (iNativeFileVerify->Open(filename, mode, params, fileserv) != result)
        OSCL_ASSERT(0);
#endif
    return result;
}

int32 OsclAsyncFile::Close()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                    (0, "OsclAsyncFile(0x%x)::Close", this));

    //stop the async read
    StartAsyncRead(false);

    //stop the thread
    if (!iHasNativeAsyncRead)
    {
        StopAsyncReadThread();
    }

    //close the duplicate file handle
    if (!iNativeFileDuplicate)
        return -1;
    int32 result = iNativeFileDuplicate->Close();

#if(VERIFY_THIS)
    if (iNativeFileVerify->Close() != result)
        OSCL_ASSERT(0);
#endif
    return result;
}

void OsclAsyncFile::StartAsyncRead(bool aStartAsyncRead)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                    (0, "OsclAsyncFile(0x%x)::StartAsyncRead %d", this, aStartAsyncRead));

    iStartAsyncRead = aStartAsyncRead;
    // Do the asynchronous read
}

uint32 OsclAsyncFile::Read(OsclAny* aBuffer, uint32 aDataSize, uint32 aNumElements)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                    (0, "OsclAsyncFile(0x%x)::Read size %d numelements %d", this, aDataSize, aNumElements));

#if(VERIFY_THIS)
    if (iVerifyCount == 1315)
        fprintf(stderr, "");
    //verify file pos before the read
    if (iNativeFileVerify->Tell() != iFilePosition)
        OSCL_ASSERT(0);
#endif

    //Pull data from the internal buffers until we get the requested amount.

    //note: there's no logic to prevent reading partial elements here.

    uint32 bytesToRead = aDataSize * aNumElements;

    uint32 bytesRead = 0;

    uint8* databuf = NULL;

    uint8* dest = (uint8*)aBuffer;

    uint32 thisread;

    uint32 nbytes;

    while (bytesToRead > 0)
    {
        //note: the "doRead" routine only handles reads of <= iTotalCacheSize,
        //so limit the read size here.
        thisread = (bytesToRead > iTotalCacheSize) ? iTotalCacheSize : bytesToRead;

        nbytes = doRead(databuf, aDataSize, thisread / aDataSize, iFilePosition);
        if (nbytes)
        {
            oscl_memcpy(dest, databuf, nbytes);
            dest += nbytes;
            bytesRead += nbytes;
            bytesToRead -= nbytes;
        }
        else
            break;//no more data or an error occurred.
    }

#if(VERIFY_THIS)
    {
        uint8* buf = (uint8*)OSCL_MALLOC(aDataSize * aNumElements);
        int32 result2 = iNativeFileVerify->Read(buf, aDataSize, aNumElements);
        if (result2 != bytesRead / aDataSize)
            OSCL_ASSERT(0);
        //verify file pos after the read
        if (iNativeFileVerify->Tell() != iFilePosition)
            OSCL_ASSERT(0);
        //verify the data
        if (oscl_memcmp(buf, aBuffer, bytesRead) != NULL)
            OSCL_ASSERT(0);
        OSCL_FREE(buf);
        iVerifyCount++;
    }
#endif

    //return number of whole elements read.
    return bytesRead / aDataSize;
}

//This routine locates the data in one of the internal buffers and returns a pointer.
//If data is not available, it will do a blocking read.
//The amount of data requested cannot exceed the internal buffer size.
//Note this returns bytes read, not number of elements read.
uint32 OsclAsyncFile::doRead(uint8 *& aBuffer1, uint32 aDataSize, uint32 aNumElements, TOsclFileOffset aOffset)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                    (0, "OsclAsyncFile(0x%x)::doRead size %d numelements %d offset %d", this, aDataSize, aNumElements, aOffset));

    //this routine only handles requests <= the individual buffer size-- it is
    //the caller's responsibility to check.
    OSCL_ASSERT(aNumElements*aDataSize <= iTotalCacheSize);

    // Clear buffers in use
    for (int32 i = 0; i < iKCacheBufferCount; i++)
    {
        iDataBufferArray[i]->CleanInUse();
    }

    // Check if we have data already in the buffer
    OsclAsyncFileBuffer* dataBuffer =  NULL;
    int32 bufferFoundId;
    uint32 bytesRead = 0;

    // Check if we have data available
    if (FindDataBuffer(dataBuffer,  bufferFoundId, aOffset, aNumElements*aDataSize))
    {
        // Mark buffer in use
        dataBuffer->SetInUse();
        // update user pointer
        OsclBuf* buff = dataBuffer->Buffer();
        OsclPtr ptrRead = buff->Des();
        aBuffer1 = const_cast<uint8*>(ptrRead.Ptr());
        // offset pointer to correct location
        aBuffer1 += (aOffset - dataBuffer->Offset());
        bytesRead = dataBuffer->Length() - (uint32)(aOffset - dataBuffer->Offset());
        // Redo queue of linked buffers
        ReOrderBuffersQueue(bufferFoundId);
    }
    else
    {
        //Data is not available-- must do a synchronous read.

        int32 ret = iNativeFileDuplicate->Seek(aOffset, Oscl_File::SEEKSET);
        if (ret != 0)
        {
            // This is not good
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OsclAsyncFile(0x%x)::doRead Seek Failed returned %d offset %d", this, ret, aOffset));
            return 0;
        }

        // reset linked data bufferarray
        iLinkedDataBufferArray.clear();

        // Get buffer to read
        OsclAsyncFileBuffer* dataBuffer;
        bool bufferAvailable = GetNextDataBuffer(dataBuffer, aOffset);
        if (!bufferAvailable)
        {
            // This case should never happen since iLinkedDataBufferArray was just closed
            OSCL_ASSERT(0);
            return 0;
        }

        dataBuffer->SetOffset(aOffset);

        // Buffer is in use
        dataBuffer->SetInUse();
        OsclBuf* readBuffer = dataBuffer->Buffer();
        OsclPtr ptrCurrentBuffer = readBuffer->Des();
        ptrCurrentBuffer.SetLength(0);

        // Do read.  Might as well fill up the entire buffer while we're at it, so
        // go ahead and read 'iTotalCacheSize'.  The caller has already verified that
        // the number of bytes requested in this read is <= 'iTotalCacheSize'.
        bytesRead = iNativeFileDuplicate->Read(ptrCurrentBuffer.Ptr(), 1, iTotalCacheSize);
        ptrCurrentBuffer.SetLength(bytesRead);

        // update data
        dataBuffer->UpdateData();

        // return value
        aBuffer1 = (uint8*)ptrCurrentBuffer.Ptr();

        iSyncFilePosition = aOffset + bytesRead;

        iLinkedDataBufferArray.push_back(dataBuffer);
    }

    // set the returned byte count to the actual bytes available from this call
    // or the amount requested, whichever is less.
    if (bytesRead > aDataSize*aNumElements)
        bytesRead = aDataSize * aNumElements;

    // Update virtual file pointer position
    iFilePosition = aOffset + bytesRead;

    // This is the position from where we read
    iLastUserFileRead = iFilePosition;

    // Continue reading
    if (iStartAsyncRead)
    {
        UpdateReading();
    }

    return bytesRead;
}

bool OsclAsyncFile::FindDataBuffer(OsclAsyncFileBuffer*& aDataBuffer, int32& aBufferId, TOsclFileOffset aOffset, int32 aSize)
{
    // Look for the requested value on the queue
    OsclAsyncFileBuffer* tmpDataBuffer;
    int32 i;
    bool found = false;
    for (i = 0; i < iKCacheBufferCount; i++)
    {
        tmpDataBuffer = iDataBufferArray[i];
        if (tmpDataBuffer->HasThisOffset(aOffset))
        {
            // we want a linked buffer
            if (IsLinkedDataBuffer(tmpDataBuffer))
            {
                found = true;
                break;
            }
            else
            {
                // check if it can be linked
                if (CanBeLinked(tmpDataBuffer))
                {
                    found = true;
                    break;
                }
            }
        }
    }

    if (!found)
        return false;

    aDataBuffer = tmpDataBuffer;

    // return id of the buffer (needed because down below we might return a different buffer than tmpDataBuffer)
    // but we still need the id
    aBufferId = tmpDataBuffer->Id();

    // Check if we have enough data to return in a single buffer
    TOsclFileOffset maxOffset = tmpDataBuffer->Offset() + tmpDataBuffer->Length();

    // the easy case
    if (aOffset + aSize <= maxOffset)
        return true;

    // Last option, check if we can concatenate required data from two buffers
    int32 availableData = (int32)(maxOffset - aOffset);

    // check again in the buffers for the remaining part of the data
    OsclAsyncFileBuffer* tmpDataBuffer2 = iDataBufferArray[0]; //Init pointer to clean-up compiler warning=MG
    for (i = 0; i < iKCacheBufferCount; i++)
    {
        tmpDataBuffer2 = iDataBufferArray[i];
        if (tmpDataBuffer2->HasThisOffset(maxOffset))
        {
            break;
        }
    }

    // extra data not found
    if (i == iKCacheBufferCount)
        return false;

    // make sure data is valid
    if (tmpDataBuffer2->Offset() != maxOffset)
        return false;

    // Set the buffer
    aDataBuffer = iDataBuffer;

    // length to copy from second buffer
    int32 lengthToCopy;
    if	(tmpDataBuffer2->Length() + availableData >= aSize)
        lengthToCopy = aSize - availableData;
    else
        return false; //shouldn't happen

    //note: could copy directly into target buffer here instead of into iDataBuffer,
    //but would take some code re-structuring.

    // extra data found, proceed to concatenate with memcpy
    OsclBuf* buffer = iDataBuffer->Buffer();
    OsclPtr ptrDes = buffer->Des();
    ptrDes.Zero();
    OsclPtrC tmpDataPtr = tmpDataBuffer->Buffer()->DesC();
    OsclPtrC temp_tmpDataPtr = tmpDataPtr.Right(availableData);
    tmpDataPtr.Set(&temp_tmpDataPtr);
    ptrDes.Append(tmpDataPtr);
    // find out the length to copy
    OsclBuf* buffer2 = tmpDataBuffer2->Buffer();
    OsclPtrC ptrDes2 = buffer2->DesC();
    OsclPtrC temp_ptrDes2 = ptrDes2.Left(lengthToCopy);
    ptrDes2.Set(&temp_ptrDes2);
    ptrDes.Append(ptrDes2);
    iDataBuffer->UpdateData();
    iDataBuffer->SetOffset(aOffset);

    return true;
}


void OsclAsyncFile::UpdateReading()
{
    // Wait for Run() to process data
    if (IsBusy())
        return;

    int32 bytesReadAhead = BytesReadAhead();

    // don't need to read more
    if (bytesReadAhead >= iKMinBytesReadAhead)
    {
        return;
    }

    TOsclFileOffset posToReadFrom = iFilePosition + bytesReadAhead;

    // check for eof. We stop reading
    if (posToReadFrom == iFileSize)
    {
        return;
    }

    // Start next read

    StartNextRead(posToReadFrom);
}

int32 OsclAsyncFile::BytesReadAhead()
{
    // Get the maximum offset of the last element in the linked	buffer array
    int32 index = iLinkedDataBufferArray.size() - 1;
    if (index == -1)
    {
        return 0;
    }

    OsclAsyncFileBuffer* tmpDataBuffer = iLinkedDataBufferArray[index];
    TOsclFileOffset maxOffset = tmpDataBuffer->Offset() + tmpDataBuffer->Length();
    int32 bytesReadAhead = (int32)(maxOffset - iLastUserFileRead);

    return bytesReadAhead;
}

int32 OsclAsyncFile::SortDataBuffers()
{
    Oscl_Vector<OsclAsyncFileBuffer*, OsclMemAllocator> tmpArray;
    // Append valid and not in use elements only
    OsclAsyncFileBuffer* tmpDataBuffer;
    uint32 i;
    for (i = 0; i < (uint32)iKCacheBufferCount; i++)
    {
        tmpDataBuffer = iDataBufferArray[i];
        if (tmpDataBuffer->IsValid())
        {
            tmpArray.push_back(tmpDataBuffer);
        }
    }

    iSortedDataBufferArray.clear();

    // sort them out
    for (i = 0; i < tmpArray.size(); i++)
    {
        if (i == 0)
            iSortedDataBufferArray.push_back(tmpArray[i]);
        else
        {
            OsclAsyncFileBuffer* tmpDataBuffer = tmpArray[i];
            bool insertedData = false;
            // check where to insert new element
            for (uint32 j = 0; j < iSortedDataBufferArray.size(); j++)
            {
                OsclAsyncFileBuffer* tmpDataBuffer2 = iSortedDataBufferArray[j];
                if (tmpDataBuffer->Offset() < tmpDataBuffer2->Offset())
                {
                    // Insert element
                    //iSortedDataBufferArray.Insert(tmpDataBuffer, j);
                    {
                        //note: there's no insert in oscl vector-- push to end then
                        //bubble down to desired spot.
                        iSortedDataBufferArray.push_back(tmpDataBuffer);
                        for (uint32 k = (uint32)iSortedDataBufferArray.size() - 1;k > j;k--)
                        {
                            OsclAsyncFileBuffer* temp = iSortedDataBufferArray[k-1];
                            iSortedDataBufferArray[k-1] = iSortedDataBufferArray[k];
                            iSortedDataBufferArray[k] = temp;
                        }
                    }
                    insertedData = true;
                    break;
                }
            }

            if (!insertedData)
            {
                iSortedDataBufferArray.push_back(tmpDataBuffer);
            }
        }

    }
    tmpArray.clear();

    return 0;
}

bool OsclAsyncFile::GetNextDataBuffer(OsclAsyncFileBuffer*& aDataBuffer, TOsclFileOffset aFilePointerToReadFrom)
{
    uint32 i;

    OSCL_UNUSED_ARG(aFilePointerToReadFrom);

    // Loop through the buffer list and find out the first buffer available for use
    OsclAsyncFileBuffer* tmpDataBuffer;
    for (i = 0; i < iDataBufferArray.size(); i++)
    {
        tmpDataBuffer = iDataBufferArray[i];
        if (!tmpDataBuffer->IsValid())
        {
            aDataBuffer = tmpDataBuffer;
            return true;
        }
    }

    // if all buffers are valid, return the next one with oldest data
    TOsclFileOffset smallerOffset = 0;
    OsclAsyncFileBuffer* dataBufferToUse = NULL;
    OsclAsyncFileBuffer* lastOptionBufferToUse = NULL;
    for (i = 0; i < iDataBufferArray.size(); i++)
    {
        tmpDataBuffer = iDataBufferArray[i];
        // Only return valid buffers
        if (tmpDataBuffer->IsInUse() || IsLinkedDataBuffer(tmpDataBuffer) || (tmpDataBuffer == iDataBufferInUse))
            continue;

        // We check if the buffer can be linked
        // if so, we rather take another one that is not of any use.
        if (CanBeLinked(tmpDataBuffer))
        {
            // we remember it just in case there's no other buffer to use
            lastOptionBufferToUse = dataBufferToUse;
            continue;
        }

        if (dataBufferToUse == NULL)
        {
            smallerOffset = tmpDataBuffer->Offset();
            dataBufferToUse = tmpDataBuffer;
        }
        else
        {
            if (tmpDataBuffer->Offset() < smallerOffset)
            {
                smallerOffset = tmpDataBuffer->Offset();
                dataBufferToUse = tmpDataBuffer;
            }
        }

    }

    // check what we have
    if (dataBufferToUse == NULL)
        dataBufferToUse = lastOptionBufferToUse;

    // If we didn't find any buffer, return false
    if (dataBufferToUse == NULL)
        return false;

    // Initialize buffer to use
    aDataBuffer = dataBufferToUse;

    return true;
}

void OsclAsyncFile::ReOrderBuffersQueue(int32 aFirstBufferId)
{
    // reset array
    iLinkedDataBufferArray.clear();

    // Sort data buffers first
    SortDataBuffers();

    // aFirstBufferId points to the index of the buffer in iDataBufferArray
    OsclAsyncFileBuffer* tmpDataBuffer = iDataBufferArray[aFirstBufferId];

    // Add the head element
    iLinkedDataBufferArray.push_back(tmpDataBuffer);

    // Look for the linked elements
    TOsclFileOffset offset = tmpDataBuffer->Offset() + tmpDataBuffer->Length();
    for (uint32 i = 0; i < iSortedDataBufferArray.size(); i++)
    {
        tmpDataBuffer = iSortedDataBufferArray[i];
        if (offset == tmpDataBuffer->Offset())
        {
            offset = tmpDataBuffer->Offset() + tmpDataBuffer->Length();
            iLinkedDataBufferArray.push_back(tmpDataBuffer);
        }
    }
}

bool OsclAsyncFile::IsLinkedDataBuffer(OsclAsyncFileBuffer* aDataBuffer)
{
    for (uint32 i = 0; i < iLinkedDataBufferArray.size(); i++)
    {
        OsclAsyncFileBuffer* tmpDataBuffer = iLinkedDataBufferArray[i];
        if (tmpDataBuffer == aDataBuffer)
        {
            return true;
        }
    }
    return false;
}

bool OsclAsyncFile::CanBeLinked(OsclAsyncFileBuffer* aDataBuffer)
{
    if (iLinkedDataBufferArray.size() == 0)
    {
        return false;
    }

    if (aDataBuffer->Offset() + aDataBuffer->Length() == iLinkedDataBufferArray[0]->Offset())
    {
        return true;
    }

    return false;
}

//The AO is invoked when the asynchronous Read request is complete.
void OsclAsyncFile::Run()
{
    iNumOfRun++;
    if (iStatus != OSCL_REQUEST_ERR_NONE)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "OsclAsyncFile(0x%x)::Run Error!!!", this));
        iNumOfRunErr++;
        return;
    }

    if (iAsyncReadThreadState == EAsyncReadNotActive)
    {
        //This is a safeguard against the situation when the thread is already stopped,
        //but the last completed request triggers the Run() execution
        return;
    }

    //update the iReadPtr descriptor length with the result of the async read.
    //Note all reads are of size 1, so we can use "num elements" interchangeably with
    //bytes here.
    if (iHasNativeAsyncRead)
        iReadPtr.SetLength(iNativeFile.GetReadAsyncNumElements());
    else
        iReadPtr.SetLength(iAsyncReadNumBytes);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                    (0, "OsclAsyncFile(0x%x)::Run numBytes %d ", this, iReadPtr.Length()));

    // First of all, update the internal file pointer
    // and the current buffer
    OsclBuf* readBuffer = iDataBufferInUse->Buffer();
    OsclPtr ptrCurrentBuffer = readBuffer->Des();
    int32 bytesRead = iReadPtr.Length();
    ptrCurrentBuffer.SetLength(ptrCurrentBuffer.Length() + bytesRead);
    iAsyncFilePosition += bytesRead;
    iDataBufferInUse->UpdateData();

    // check how many bytes ahead of the user file position we have
    // bool seekDone = false;
    int32 bytesReadAhead = BytesReadAhead();

    // next position to read from
    TOsclFileOffset posToReadFrom = iLastUserFileRead + bytesReadAhead;

    // check for eof. We stop reading
    if (posToReadFrom == iFileSize)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "OsclAsyncFile(0x%x)::Run Reached end of file ", this));
        return;
    }

    // don't need to read more for now
    if (bytesReadAhead >= iKMinBytesReadAhead)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "OsclAsyncFile(0x%x)::Run Reached the read-ahead count", this));
        return;
    }

    // Start next read
    StartNextRead(posToReadFrom);
}

void OsclAsyncFile::DoCancel()
{
    if (!iHasNativeAsyncRead)
    {
        if (iAsyncReadThreadState == EAsyncReadNotActive)
        {
            //in case thread exited with some request active, this will
            //complete it
            OsclActiveObject::DoCancel();
        }
        else
        {
            //in this case, thread is active.  since there's no way to
            //interrupt the thread's blocking read call, just do nothing
            //here, then scheduler will wait on request completion.
        }
    }
}

void OsclAsyncFile::StartNextRead(TOsclFileOffset aPosToReadFrom)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                    (0, "OsclAsyncFile(0x%x)::StartNextRead pos %d ", this, aPosToReadFrom));

    // Determine from which file pointer we're going to read
    if (aPosToReadFrom == iAsyncFilePosition)
    {
        // just continue reading
        OsclBuf* readBuffer = iDataBufferInUse->Buffer();
        OsclPtr ptrCurrentBuffer = readBuffer->Des();
        bool validBuffer = true;
        // Extra validation. This should never be false, but let's check it
        if (iDataBufferInUse->Offset() + iDataBufferInUse->Length() !=  aPosToReadFrom)
        {
            validBuffer = false;
        }

        // Check if we need a new buffer
        if (validBuffer && (iTotalCacheSize - ptrCurrentBuffer.Length() < (uint32)iKAsyncReadBufferSize))
        {
            // This is the case where we could get NULL if
            // we're trying to read ahead more data than what the
            // buffers can hold. We just add some protection in case
            // someone makes a wrong configuration
            bool availableBuffer = GetNextDataBuffer(iDataBufferInUse, aPosToReadFrom);
            if (!availableBuffer)
            {
                return;
            }
            // we link the new buffer as soon as we get it
            iLinkedDataBufferArray.push_back(iDataBufferInUse);
            iDataBufferInUse->SetOffset(aPosToReadFrom);
            OsclBuf* newReadBuffer = iDataBufferInUse->Buffer();
            OsclPtr ptrNewReadBuffer = newReadBuffer->Des();
            ptrNewReadBuffer.SetLength(0);
            iDataBufferInUse->UpdateData();
            iReadPtr.Set((uint8*)ptrNewReadBuffer.Ptr(), 0, iKAsyncReadBufferSize);
        }
        else
        {
            iReadPtr.Set((uint8*)(ptrCurrentBuffer.Ptr() + ptrCurrentBuffer.Length()), 0, iKAsyncReadBufferSize);
        }
    }
    else
        if (aPosToReadFrom == iSyncFilePosition)
        {
            // update the async file pointer to the desired location
            int32 result = iNativeFile.Seek(aPosToReadFrom, Oscl_File::SEEKSET);
            if (result != 0)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OsclAsyncFile(0x%x)::StartNextRead Seek failed, return %d, offset %d ", this, result, aPosToReadFrom));
                return;
            }

            // we need to swap the file pointers
            TOsclFileOffset tmpPosition = iSyncFilePosition;
            iSyncFilePosition = iAsyncFilePosition;
            iAsyncFilePosition = tmpPosition;

            // We need a new buffer
            bool availableBuffer = GetNextDataBuffer(iDataBufferInUse, aPosToReadFrom);
            if (!availableBuffer)
            {
                // This is an error condition. There should always be a buffer available
                OSCL_ASSERT(0);
                return;
            }
            // we link the new buffer as soon as we get it
            iLinkedDataBufferArray.push_back(iDataBufferInUse);
            iDataBufferInUse->SetOffset(aPosToReadFrom);
            // Initialize the read pointer (iReadPtr)
            OsclBuf* readBuffer = iDataBufferInUse->Buffer();
            OsclPtr ptrCurrentBuffer = readBuffer->Des();
            // reset length
            ptrCurrentBuffer.SetLength(0);
            iDataBufferInUse->UpdateData();
            iReadPtr.Set((uint8*)ptrCurrentBuffer.Ptr(), 0, iKAsyncReadBufferSize);
        }
        else
        {
            // This case should never happen.. check it out
            OSCL_ASSERT(0);
            return;
        }

    // Issue the asynchronous read request.

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                    (0, "OsclAsyncFile(0x%x)::StartNextRead Issuing async read, size %d ", this, iKAsyncReadBufferSize));

    if (iHasNativeAsyncRead)
    {
        //Activate the AO that will handle read completion.
        PendForExec();

        //Start the native async read operation
        int32 result = iNativeFile.ReadAsync(iReadPtr.Ptr(), 1, iKAsyncReadBufferSize, StatusRef());

        //if it failed to start, then cancel the request with an error.
        if (result != 0)
            PendComplete(OSCL_REQUEST_ERR_GENERAL);
    }
    else
    {
        //Start the non-native async read operation.

        StartNonNativeAsyncRead();
        //the AO will run when read is complete.
    }
}

int32 OsclAsyncFile::Seek(TOsclFileOffset offset, Oscl_File::seek_type origin)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                    (0, "OsclAsyncFile(0x%x)::Seek offset %d origin %d", this, offset, origin));

    if (origin == Oscl_File::SEEKCUR)
    {
        iFilePosition += offset;
    }
    else
        if (origin == Oscl_File::SEEKSET)
        {
            iFilePosition = offset;
        }
        else
            if (origin == Oscl_File::SEEKEND)
            {
                iFilePosition = iFileSize + offset;
            }

    //some sanity checks.
    OSCL_ASSERT(iFilePosition >= 0);
    OSCL_ASSERT(iFilePosition <= iFileSize);

#if(VERIFY_THIS)
    if (iNativeFileVerify->Seek(offset, origin) != 0)
        OSCL_ASSERT(0);
    //also verify current position
    if (iNativeFileVerify->Tell() != iFilePosition)
        OSCL_ASSERT(0);
#endif
    return 0;
}


TOsclFileOffset OsclAsyncFile::Tell()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                    (0, "OsclAsyncFile(0x%x)::Tell pos %d ", this, iFilePosition));

#if(VERIFY_THIS)
    if (iNativeFileVerify->Tell() != iFilePosition)
        OSCL_ASSERT(0);
#endif
    return iFilePosition;
}

int32  OsclAsyncFile::EndOfFile()
{
    int32 result = (iFilePosition >= iFileSize) ? 1 : 0;

#if(VERIFY_THIS)
    if (iNativeFileVerify->EndOfFile() != result)
        OSCL_ASSERT(0);
#endif

    return result;
}

TOsclFileOffset OsclAsyncFile::Size()
{
#if(VERIFY_THIS)
    if (iNativeFileVerify->Size() != iFileSize)
        OSCL_ASSERT(0);
#endif

    return iFileSize;
}

// ==================   Non-Native async read implementation  ==========================  //

void OsclAsyncFile::LaunchAsyncReadThread()
{
    if (iAsyncReadThreadState != EAsyncReadActive)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "OsclAsyncFile(0x%x)::LaunchAsyncReadThread starting thread ", this));

        iAsyncReadSem.Create(0);
        iAsyncReadExitSem.Create(0);
        OsclThread thread;
        OsclProcStatus::eOsclProcError status = thread.Create(iAsyncReadThreadFunc, 4096, (TOsclThreadFuncArg)this);
        if (status == OsclProcStatus::SUCCESS_ERROR)
        {
            //Thread was successfuly started, update thread's state
            iAsyncReadThreadState = EAsyncReadActive;
        }
    }
}

void OsclAsyncFile::StopAsyncReadThread()
{
    if (iAsyncReadThreadState == EAsyncReadActive)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "OsclAsyncFile(0x%x)::StopAsyncReadThread stopping thread ", this));

        //signal the thread to exit & wake it up.
        iAsyncReadThreadExitFlag = true;
        iAsyncReadSem.Signal();

        //wait on thread to exit so we can reset the sems safely
        iAsyncReadExitSem.Wait();
        //Thread is terminated, update thread's state
        iAsyncReadThreadState = EAsyncReadNotActive;

        iAsyncReadSem.Close();
        iAsyncReadExitSem.Close();
    }
}

void OsclAsyncFile::StartNonNativeAsyncRead()
{
    //note: we assume the read requests are nicely serialized here,
    //so there's no handling for overlapping requests.

    //Activate the AO that will wait on read completion.
    PendForExec();

    //wake up the thread
    iAsyncReadSem.Signal();
}

void OsclAsyncFile::InThread()
{
    while (iAsyncReadThreadExitFlag != true)
    {
        //wait for the signal
        iAsyncReadSem.Wait();

        //see if it's a close request
        if (iAsyncReadThreadExitFlag == true)
        {
            break;
        }

        //issue the read.
        iAsyncReadNumBytes = iNativeFile.Read(iReadPtr.Ptr(), 1, iKAsyncReadBufferSize);

        //complete the request.
        if (IsAdded() && iStatus == OSCL_REQUEST_PENDING)
            PendComplete(OSCL_REQUEST_ERR_NONE);
    }

    // reset the thread exit flag
    iAsyncReadThreadExitFlag = false;
    //signal that thread is exiting.
    iAsyncReadExitSem.Signal();
}

//static thread routine.
TOsclThreadFuncRet OSCL_THREAD_DECL OsclAsyncFile::iAsyncReadThreadFunc(TOsclThreadFuncArg aArg)
{
    OsclAsyncFile* This = (OsclAsyncFile*)aArg;

    This->InThread();

    return 0;
}
