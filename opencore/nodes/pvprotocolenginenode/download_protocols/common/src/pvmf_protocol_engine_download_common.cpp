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
#include "pvmf_protocol_engine_download_common.h"

////////////////////////////////////////////////////////////////////////////////////
//////	DownloadState related implementation
////////////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF int32 DownloadState::processMicroStateSendRequestPreCheck()
{
    if (!iCfgFile.GetRep()) return PROCESS_INPUT_OUTPUT_NOT_READY;
    return ProtocolState::processMicroStateSendRequestPreCheck();
}

OSCL_EXPORT_REF int32 DownloadState::processMicroStateGetResponsePreCheck()
{
    int32 status = ProtocolState::processMicroStateGetResponsePreCheck();
    if (status != PROCESS_SUCCESS) return status;

    // reset
    iOutputDataQueue.clear();

    // register observer and data queue to be used in iParser
    iParser->registerObserverAndOutputQueue(this, &iOutputDataQueue);

    return PROCESS_SUCCESS;
}

OSCL_EXPORT_REF void DownloadState::setRequestBasics()
{
    iComposer->setMethod(HTTP_METHOD_GET);
    //iComposer->setVersion(HTTP_V1_1);
    iComposer->setVersion((HTTPVersion)iCfgFile->getHttpVersion());
    StrPtrLen uri((iURI.getURI()).get_cstr(), (iURI.getURI()).get_size());
    iComposer->setURI(uri);
}

// For composing a request, only need to override this function
OSCL_EXPORT_REF bool DownloadState::setHeaderFields()
{
    // set fields
    OSCL_FastString fieldKeyString(_STRLIT_CHAR("Host"));
    StrCSumPtrLen fieldKey(fieldKeyString.get_cstr());
    if (!iComposer->setField(fieldKey, iURI.getHost().get_cstr())) return false;

    fieldKeyString.set((OSCL_String::chartype*)_STRLIT_CHAR("User-Agent"), 16);
    fieldKey.setPtrLen(fieldKeyString.get_str(), fieldKeyString.get_size());
    if (!iComposer->setField(fieldKey, (iCfgFile->GetUserAgent()).get_cstr())) return false;

    fieldKeyString.set((OSCL_String::chartype*)_STRLIT_CHAR("Connection"), 16);
    OSCL_FastString fieldValueString(_STRLIT_CHAR("Keep-Alive"));
    fieldKey.setPtrLen(fieldKeyString.get_cstr(), fieldKeyString.get_size());
    if (!iComposer->setField(fieldKey, fieldValueString.get_cstr())) return false;

    return true;
}

// shared routine for all the download protocols
OSCL_EXPORT_REF int32 DownloadState::checkParsingStatus(int32 parsingStatus)
{
    if (parsingStatus == HttpParsingBasicObject::PARSE_CONTENT_RANGE_INFO_NOT_MATCH)
    {
        return PROCESS_CONTENT_RANGE_INFO_NOT_MATCH;
    }
    else if (parsingStatus == HttpParsingBasicObject::PARSE_CONTENT_LENGTH_NOT_MATCH)
    {
        return PROCESS_CONTENT_LENGTH_NOT_MATCH;
    }

    return ProtocolState::checkParsingStatus(parsingStatus);
}

OSCL_EXPORT_REF int32 DownloadState::updateDownloadStatistics()
{
    // update current file size in config file
    uint32 downloadSize = iParser->getDownloadSize();
    bool aFirstDownloadFromFileBeginning = (downloadSize > 0 && iCfgFile->GetCurrentFileSize() == 0);
    if (downloadSize > iCfgFile->GetCurrentFileSize())
    {
        iCfgFile->SetCurrentFileSize(downloadSize);
        // set ContentLengthFlag to ConfigFile object
        if (!iSetContentLengthFlagtoConfigFileObject)
        {
            iCfgFile->setHasContentLengthFlag((iParser->getContentLength() > 0));
            iSetContentLengthFlagtoConfigFileObject = true;
        }
    }

    // update total file size in config file
    uint32 contentLength = iParser->getContentLength();
    if (iCfgFile->GetOverallFileSize() == 0 ||
            (iCfgFile->GetOverallFileSize() != contentLength && contentLength > 0))
    {
        iCfgFile->SetOverallFileSize(contentLength);
    }
    //else if(contentLength == 0 && downloadSize >= iCfgFile->GetOverallFileSize()) {
    else if (contentLength == 0)
    {
        //iCfgFile->SetOverallFileSize(downloadSize);
        if (downloadSize > iCfgFile->GetMaxAllowedFileSize())
        {
            // file has to be truncated due to request size limition
            iParser->setDownloadSize(iCfgFile->GetMaxAllowedFileSize());
            iCfgFile->SetOverallFileSize(iCfgFile->GetMaxAllowedFileSize());
            iCfgFile->SetCurrentFileSize(iCfgFile->GetMaxAllowedFileSize());
            return PROCESS_SUCCESS_END_OF_MESSAGE_TRUNCATED;
        }
    }

    // save to config file for the first download from beginning of the file
    if (aFirstDownloadFromFileBeginning) saveConfig();
    return PROCESS_SUCCESS;
}

OSCL_EXPORT_REF void DownloadState::saveConfig()
{
    iCfgFile->SaveConfig();
}

