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

#ifndef PVMF_CPMPLUGIN_KMJ_OMA1_DATA_ACCESS_H
#define PVMF_CPMPLUGIN_KMJ_OMA1_DATA_ACCESS_H

class DrmPluginDataAccess
{
public:
    typedef enum
    {
        ACCESS_FILE,   // access data through file
      ACCESS_BUFFER  // access data through buffer
    }DataAccessMode;

    /**
       * Get the data size in bytes.
       *
       * @return - The size of the data, or -1 on error.
       */
    int32_t GetDataLen();

    /**
     * The data Seek operation
     * Sets the position for data access
     *
     * @param offset offset from the start
     *
     * @return returns 0 on success, or -1 on error.
     */
    int32_t SeekData(int32_t offset);

      /**
       * The data Read operation
       *
       * @param buf  pointer to buffer to store read data
       * @param bufLen  dat size in bytes need to read
       *
       * @return returns the number of actually read, or <= 0 on error
       */
    int32_t ReadData(uint8_t* buf,int32_t bufLen);

    // constuctor
    DrmPluginDataAccess( PVFile* dataFile,
                         DataAccessMode mode = ACCESS_FILE,
                         char* buf = NULL,
                         int len = 0)
        : pvfile(dataFile),
          accessMode(mode),
          dataBuf(buf),
          dataLen(len),
          dataPos(buf)
    {}

private:
    // access data mode, now support file and buffer
    DataAccessMode accessMode;

    // access data from file
    PVFile* pvfile;

    // access data from buffer
    char* dataBuf;
    int dataLen;
    char* dataPos;
};

/**
 * Get the data size in bytes.
 *
 * @param dataHandle the data handle from which get data len
 *
 * @return - The size of the data, or -1 on error.
 */
extern int32_t DrmPluginGetDataLen(int32_t dataHandle);

/**
 * The data Seek operation
 * Sets the position for data access
 *
 * @param dataHandle the data handle from which seek data
 * @param offset offset from the start
 *
 * @return returns 0 on success, or -1 on error.
 */
extern int32_t DrmPluginSeekData( int32_t dataHandle,int32_t dataOffset);

/**
 * The data Read operation
 *
 * @param dataHandle the data handle from which read data
 * @param buf  pointer to buffer to store read data
 * @param bufLen  dat size in bytes need to read
 *
 * @return returns the number of actually read, or <= 0 on error
 */
extern int32_t DrmPluginReadData( int32_t dataHandle,uint8_t* buf,int32_t bufLen);

#endif

