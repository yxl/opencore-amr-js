/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "oscl_base.h"
#include "oscl_types.h"
#include "pvfile.h"
#include "oscl_file_io.h"
#include "pvmf_cpmplugin_kmj_oma1_data_access.h"

int32_t DrmPluginDataAccess::GetDataLen()
{
    if(ACCESS_FILE == accessMode)
    {
        pvfile->Seek(0,Oscl_File::SEEKEND);

        int32_t dataLen = pvfile->Tell();
        if(-1 == dataLen) // get content len failed
        {
            pvfile->Close();
            return -1;
        }

        pvfile->Seek(0,Oscl_File::SEEKSET);
        return dataLen;
    }
    else if(ACCESS_BUFFER == accessMode)
    {
        return dataLen;
    }
    else
    {
        return -1;
    }
}

int32_t DrmPluginDataAccess::SeekData(int32_t offset)
{
    if(ACCESS_FILE == accessMode)
    {
        return pvfile->Seek(offset,Oscl_File::SEEKSET);
    }
    else if(ACCESS_BUFFER == accessMode)
    {
        if(offset < 0)
        {
            return -1;
        }

        if((offset > dataLen) || (dataPos - dataBuf) + offset > dataLen)
        {
            dataPos = dataBuf + dataLen;
        }

        dataPos = dataBuf + offset;
        return 0;
    }
    else
    {
        return -1;
    }
}

int32_t DrmPluginDataAccess::ReadData(uint8_t* buf,int32_t bufLen)
{
    if(ACCESS_FILE == accessMode)
    {
        return pvfile->Read(buf,sizeof(uint8_t),bufLen);
    }
    else if(ACCESS_BUFFER == accessMode)
    {
        if((dataPos - dataBuf) >= dataLen)
        {
            return 0;
        }

        int32_t readDataLen;

        if(((dataPos - dataBuf) + bufLen) <= dataLen)
        {
            readDataLen = bufLen;
        }
        else
        {
            readDataLen = dataLen - (dataPos - dataBuf);
        }

        memcpy(buf,dataPos,readDataLen);

        return readDataLen;
    }
    else
    {
        return 0;
    }
}

int32_t DrmPluginGetDataLen(int32_t dataHandle)
{
    DrmPluginDataAccess* dataAccess = (DrmPluginDataAccess*)dataHandle;
    return dataAccess->GetDataLen();
}

int32_t DrmPluginSeekData( int32_t dataHandle,int32_t dataOffset)
{
    DrmPluginDataAccess* dataAccess = (DrmPluginDataAccess*)dataHandle;
    return dataAccess->SeekData(dataOffset);
}

int32_t DrmPluginReadData( int32_t dataHandle,uint8_t* buf,int32_t bufLen)
{
    DrmPluginDataAccess* dataAccess = (DrmPluginDataAccess*)dataHandle;
    return dataAccess->ReadData(buf,bufLen);
}

