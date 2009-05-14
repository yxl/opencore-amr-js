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
#include "pvmf_protocol_engine_progressive_download.h"

//////	ProgressiveDownloadState_HEAD implementation ////////////////////////////
OSCL_EXPORT_REF void ProgressiveDownloadState_HEAD::setRequestBasics()
{
    iComposer->setMethod(HTTP_METHOD_HEAD);
    //iComposer->setVersion(HTTP_V1_1);
    iComposer->setVersion((HTTPVersion)iCfgFile->getHttpVersion());
    StrPtrLen uri((iURI.getURI()).get_cstr(), (iURI.getURI()).get_size());
    iComposer->setURI(uri);
}

OSCL_EXPORT_REF bool ProgressiveDownloadState_HEAD::setHeaderFields()
{
    if (!DownloadState::setHeaderFields()) return false;
    if (!ProtocolState::constructAuthenHeader(iCfgFile->GetUserId(), iCfgFile->GetUserAuth())) return false;

    return setExtensionFields(iCfgFile->getExtensionHeaderKeys(),
                              iCfgFile->getExtensionHeaderValues(),
                              iCfgFile->getHTTPMethodMasksForExtensionHeader(),
                              iCfgFile->getExtensionHeadersPurgeOnRedirect(),
                              HTTP_METHOD_HEAD);
}

OSCL_EXPORT_REF int32 ProgressiveDownloadState_HEAD::checkParsingStatus(int32 parsingStatus)
{
    if (parsingStatus == HttpParsingBasicObject::PARSE_SUCCESS_END_OF_INPUT && iParser->isHttpHeaderParsed())
        return PROCESS_SUCCESS_END_OF_MESSAGE;

    return ProtocolState::checkParsingStatus(parsingStatus);
}

OSCL_EXPORT_REF int32 ProgressiveDownloadState_HEAD::OutputDataAvailable(OUTPUT_DATA_QUEUE *aOutputQueue, const bool isHttpHeader)
{
    if (isHttpHeader)
    {
        iDataSideInfo.set(ProtocolEngineOutputDataType_HttpHeader);
        iObserver->OutputDataAvailable(*aOutputQueue, iDataSideInfo);
    }
    return HttpParsingBasicObject::PARSE_SUCCESS;
}

//////	ProgressiveDownloadState_GET implementation ////////////////////////////
OSCL_EXPORT_REF int32 ProgressiveDownloadState_GET::processMicroStateGetResponsePreCheck()
{
    int32 status = DownloadState::processMicroStateGetResponsePreCheck();
    if (status != PROCESS_SUCCESS) return status;

    // set the existing download size if this is resume download
    iParser->setDownloadSize(iCfgFile->GetCurrentFileSize());

    return PROCESS_SUCCESS;
}

OSCL_EXPORT_REF bool ProgressiveDownloadState_GET::setHeaderFields()
{
    // check range header
    if (!setRangeHeaderFields()) return false;

    // set authentication header and common headers
    if (!ProtocolState::constructAuthenHeader(iCfgFile->GetUserId(), iCfgFile->GetUserAuth())) return false;
    if (!DownloadState::setHeaderFields()) return false;

    // change "Connection" field
    StrCSumPtrLen connectionKey = "Connection";
    char *nullPtr = NULL; // remove "Connection" field
    if (!iComposer->setField(connectionKey, nullPtr)) return false;
    // reset "Connection: Close"
    StrPtrLen  connectionValue = "Close";
    if (!iComposer->setField(connectionKey, &connectionValue)) return false;


    return setExtensionFields(iCfgFile->getExtensionHeaderKeys(),
                              iCfgFile->getExtensionHeaderValues(),
                              iCfgFile->getHTTPMethodMasksForExtensionHeader(),
                              iCfgFile->getExtensionHeadersPurgeOnRedirect());
}

OSCL_EXPORT_REF bool ProgressiveDownloadState_GET::setRangeHeaderFields()
{
    if (iRangeHeaderSupported)
    {
        // only send Range header for previous non-zero bytes position.
        // Some server may not like this, Range: bytes=0-
        if (iCfgFile->GetCurrentFileSize() > 0 && iCfgFile->GetOverallFileSize() > 0)
        {
            StrCSumPtrLen rangeKey = "Range";
            char buffer[64];
            oscl_snprintf(buffer, 64, "bytes=%d-%d", iCfgFile->GetCurrentFileSize(), iCfgFile->GetOverallFileSize());
            LOGINFODATAPATH((0, "ProgressiveDownloadState_GET::setHeaderFields(), Range: bytes=%d-", iCfgFile->GetCurrentFileSize()));
            if (!iComposer->setField(rangeKey, buffer)) return false;
        }
    }
    return true;
}

OSCL_EXPORT_REF int32 ProgressiveDownloadState_GET::updateDownloadStatistics()
{
    int32 status = DownloadState::updateDownloadStatistics();
    if (status == PROCESS_SUCCESS_END_OF_MESSAGE_TRUNCATED)
    {
        iSendEndOfMessageTruncate = true;
    }
    return status;
}


OSCL_EXPORT_REF int32 ProgressiveDownloadState_GET::checkParsingStatus(int32 parsingStatus)
{
    // EOS means connection is down, and can be treated as download complete
    if (parsingStatus == HttpParsingBasicObject::PARSE_EOS_INPUT_DATA)
    {
        if (iParser->getDownloadSize() > 0 && iParser->isDownloadReallyHappen())
        {
            iCfgFile->SetCurrentFileSize(iParser->getDownloadSize());
            if (iParser->getContentLength() == 0) iCfgFile->SetOverallFileSize(iParser->getDownloadSize());
            LOGINFODATAPATH((0, "ProgressiveDownloadState_GET::checkParsingStatus(), GOT EOS and COMPLETE DOWNLOAD, downloadSize=%d, contentLength=%d, isDownloadHappen=%d",
                             iParser->getDownloadSize(), iParser->getContentLength(), (int32)iParser->isDownloadReallyHappen()));
            return PROCESS_SUCCESS_END_OF_MESSAGE_BY_SERVER_DISCONNECT;
        }
    }

    // download complete with truncation
    if (iSendEndOfMessageTruncate)
    {
        iSendEndOfMessageTruncate = false;
        return PROCESS_SUCCESS_END_OF_MESSAGE_TRUNCATED;
    }

    return DownloadState::checkParsingStatus(parsingStatus);
}


// From HttpParsingBasicObjectObserver
OSCL_EXPORT_REF int32 ProgressiveDownloadState_GET::OutputDataAvailable(OUTPUT_DATA_QUEUE *aOutputQueue, const bool isHttpHeader)
{
    if (isHttpHeader)
    {
        int32 status = checkContentInfoMatchingForResumeDownload();
        if (status != HttpParsingBasicObject::PARSE_SUCCESS) return status;

        iDataSideInfo.set(ProtocolEngineOutputDataType_HttpHeader);
        iObserver->OutputDataAvailable(*aOutputQueue, iDataSideInfo);
    }
    else  	// output data to data stream object
    {
        if (iParser->getDownloadSize() > iCfgFile->GetCurrentFileSize())
        {
            updateOutputDataQueue(aOutputQueue); // aOutputQueue could have the partial valid data for resume download and trucated content case
            iDataSideInfo.set(ProtocolEngineOutputDataType_NormalData);
            iObserver->OutputDataAvailable(*aOutputQueue, iDataSideInfo);
            return updateDownloadStatistics(); // could return PROCESS_SUCCESS_END_OF_MESSAGE_TRUNCATED
        }
    }
    return HttpParsingBasicObject::PARSE_SUCCESS;
}

OSCL_EXPORT_REF int32 ProgressiveDownloadState_GET::checkContentInfoMatchingForResumeDownload()
{
    if (iCfgFile->IsNewSession()) return HttpParsingBasicObject::PARSE_SUCCESS;
    uint32 prevOverallFileSize = iCfgFile->GetOverallFileSize();
    if (iCfgFile->GetOverallFileSize() == iCfgFile->GetMaxAllowedFileSize() && !iCfgFile->HasContentLength())
    {
        prevOverallFileSize = 0; // no content-length for the previous download
    }
    int32 status = iParser->isNewContentRangeInfoMatchingCurrentOne(prevOverallFileSize);
    // Get internal download size synced up with new content-range info
    iParser->setDownloadSize();
    return status;
}

OSCL_EXPORT_REF void ProgressiveDownloadState_GET::updateOutputDataQueue(OUTPUT_DATA_QUEUE *aOutputQueue)
{
    // get start fragment especially for resume download case
    bool aUseAllNewDownloadData;
    uint32 aStartFragNo = 0, aStartFragOffset = 0;
    getStartFragmentInNewDownloadData(*aOutputQueue, aUseAllNewDownloadData, aStartFragNo, aStartFragOffset);
    if (aUseAllNewDownloadData) return;

    LOGINFODATAPATH((0, "ProgressiveDownloadState_GET::updateOutputDataQueue()->getStartFragmentInNewDownloadData() : aOutputQueue->size=%d, aStartFragNo=%d, aStartFragOffset=%d",
                     aOutputQueue->size(), aStartFragNo, aStartFragOffset));

    LOGINFODATAPATH((0, "ProgressiveDownloadState_GET::updateOutputDataQueue()->getStartFragmentInNewDownloadData() : downloadSize=%d, currFileSize=%d",
                     iParser->getDownloadSize(), iCfgFile->GetCurrentFileSize()));

    // process start fragment
    if (!(aStartFragNo == 0 && aStartFragOffset == 0))   // exist offset
    {
        OsclMemoryFragment memFrag;
        uint8 *startPtr = (uint8*)((*aOutputQueue)[aStartFragNo].getMemFragPtr()) + aStartFragOffset;
        memFrag.ptr = (OsclAny*)startPtr;
        memFrag.len = (*aOutputQueue)[aStartFragNo].getMemFragSize() - aStartFragOffset;
        OsclRefCounter *refcnt = (*aOutputQueue)[aStartFragNo].getRefCounter();
        OsclRefCounterMemFrag refCountMemFrag = OsclRefCounterMemFrag(memFrag, refcnt, memFrag.len);
        refcnt->addRef(); // manually add reference counter since there will be vector push_back happens.

        for (uint32 i = 0; i <= aStartFragNo; i++)
        {
            aOutputQueue->erase(aOutputQueue->begin());
        }
        if (memFrag.len > 0) aOutputQueue->push_front(refCountMemFrag);

        LOGINFODATAPATH((0, "ProgressiveDownloadState_GET::updateOutputDataQueue() after processing start fragment: aOutputQueue->size=%d",	aOutputQueue->size()));
    }

    // get end fragment especially for truncated content case
    uint32 aEndFragNo = 0, aEndFragValidLen = 0;
    getEndFragmentInNewDownloadData(*aOutputQueue, aEndFragNo, aEndFragValidLen);

    LOGINFODATAPATH((0, "ProgressiveDownloadState_GET::updateOutputDataQueue()->getEndFragmentInNewDownloadData() : aOutputQueue->size=%d, aEndFragNo=%d, aEndFragValidLen=%d",
                     aOutputQueue->size(), aEndFragNo, aEndFragValidLen));

    LOGINFODATAPATH((0, "ProgressiveDownloadState_GET::updateOutputDataQueue()->getStartFragmentInNewDownloadData() : downloadSize=%d, overallFileSize=%d",
                     iParser->getDownloadSize(), iCfgFile->GetOverallFileSize()));

    // process end fragment
    if (!(aEndFragNo == aOutputQueue->size() - 1 &&
            aEndFragValidLen == (*aOutputQueue)[aEndFragNo].getMemFragSize()))
    {
        OsclMemoryFragment memFrag;
        memFrag.ptr = (*aOutputQueue)[aEndFragNo].getMemFragPtr();
        memFrag.len = aEndFragValidLen;
        OsclRefCounter *refcnt = (*aOutputQueue)[aEndFragNo].getRefCounter();
        OsclRefCounterMemFrag refCountMemFrag = OsclRefCounterMemFrag(memFrag, refcnt, memFrag.len);
        refcnt->addRef(); // manually add reference counter since there will be vector push_back happens.

        for (int32 j = (int32)aOutputQueue->size() - 1; j >= (int32)aEndFragNo; j--)
        {
            aOutputQueue->erase(&(aOutputQueue->back()));
        }

        aOutputQueue->push_back(refCountMemFrag);

        LOGINFODATAPATH((0, "ProgressiveDownloadState_GET::updateOutputDataQueue() after processing end fragment: aOutputQueue->size=%d", aOutputQueue->size()));
    }
}

OSCL_EXPORT_REF void ProgressiveDownloadState_GET::getStartFragmentInNewDownloadData(OUTPUT_DATA_QUEUE &aOutputQueue,
        bool &aUseAllNewDownloadData,
        uint32 &aStartFragNo,
        uint32 &aStartFragOffset)
{
    aUseAllNewDownloadData = false;
    aStartFragNo = aStartFragOffset = 0;

    uint32 validSize = iParser->getDownloadSize() - iCfgFile->GetCurrentFileSize();

    uint32 totalSize = 0, prevTotalSize = 0;
    for (uint32 i = 0; i < aOutputQueue.size(); i++)
    {
        prevTotalSize = totalSize;
        totalSize += aOutputQueue[i].getMemFragSize();
        if (prevTotalSize <= validSize && validSize < totalSize)
        {
            if (validSize < totalSize && i < aOutputQueue.size() - 1)
            {
                aStartFragNo = i;
                aStartFragOffset = validSize - prevTotalSize;
                return;
            }
        }
    }

    aUseAllNewDownloadData = (validSize == totalSize) &
                             (iParser->getDownloadSize() <= iCfgFile->GetOverallFileSize());
}

OSCL_EXPORT_REF void ProgressiveDownloadState_GET::getEndFragmentInNewDownloadData(OUTPUT_DATA_QUEUE &aOutputQueue,
        uint32 &aEndFragNo,
        uint32 &aEndFragValidLen)
{
    aEndFragNo = aOutputQueue.size() - 1;
    aEndFragValidLen = aOutputQueue[aEndFragNo].getMemFragSize();

    if (iParser->getDownloadSize() > iCfgFile->GetOverallFileSize())
    {
        uint32 extraSize = iParser->getDownloadSize() - iCfgFile->GetOverallFileSize();
        uint32 reduceSize = 0, prevReduceSize = 0;
        for (int32 i = aOutputQueue.size() - 1; i >= 0; i--)
        {
            prevReduceSize = reduceSize;
            reduceSize += aOutputQueue[i].getMemFragSize();
            if (prevReduceSize <= extraSize && extraSize < reduceSize)
            {
                aEndFragNo = i;
                aEndFragValidLen = reduceSize - extraSize;
                return;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
//////	ProgressiveStreamingState implementation
////////////////////////////////////////////////////////////////////////////////////

OSCL_EXPORT_REF int32 ProgressiveStreamingState_GET::checkParsingStatus(int32 parsingStatus)
{
    // download complete with truncation
    if (iSendEndOfMessageTruncate)
    {
        iSendEndOfMessageTruncate = false;
        return PROCESS_SUCCESS_END_OF_MESSAGE_TRUNCATED;
    }

    return DownloadState::checkParsingStatus(parsingStatus);
}

