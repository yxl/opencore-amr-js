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
#ifndef PV_AVIFILE_STATUS
#define PV_AVIFILE_STATUS

#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif

#define PVAVIFILE_LOGERROR(m)	 PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iLogger,PVLOGMSG_ERR,m)
#define PVAVIFILE_LOGINFO(m)	 PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iLogger,PVLOGMSG_INFO,m)

typedef enum
{
    PV_AVI_FILE_PARSER_ERROR_UNKNOWN             = -1,
    PV_AVI_FILE_PARSER_SUCCESS                   = 0,
    PV_AVI_FILE_PARSER_FILE_OPEN_ERROR           = 1,
    PV_AVI_FILE_PARSER_INSUFFICIENT_MEMORY       = 2,
    PV_AVI_FILE_PARSER_WRONG_CHUNK               = 3,
    PV_AVI_FILE_PARSER_WRONG_CHUNK_SIZE          = 4,
    PV_AVI_FILE_PARSER_WRONG_FILE                = 5,
    PV_AVI_FILE_PARSER_WRONG_SIZE                = 6,
    PV_AVI_FILE_PARSER_READ_ERROR                = 7,
    PV_AVI_FILE_PARSER_ERROR_NUM_STREAM          = 8,
    PV_AVI_FILE_PARSER_ERROR_STREAM_TYPE_UNKNOWN = 9,
    PV_AVI_FILE_PARSER_NO_INDEX_CHUNK            = 10,
    PV_AVI_FILE_PARSER_EOF_REACHED               = 11,  //End of file
    PV_AVI_FILE_PARSER_EOS_REACHED               = 12,  //End of stream
    PV_AVI_FILE_PARSER_USE_INDX_TBL              = 13,
    PV_AVI_FILE_PARSER_BYTE_COUNT_ERROR          = 14,
    PV_AVI_FILE_PARSER_UNSUPPORTED_CHUNK         = 15,
    PV_AVI_FILE_PARSER_ERROR_WRONG_STREAM_NUM    = 16,
    PV_AVI_FILE_PARSER_WRONG_OFFSET              = 17,
    PV_AVI_FILE_PARSER_NO_OFFSET_FOUND           = 18,
    PV_AVI_FILE_PARSER_WRONG_BIT_COUNT           = 19,
    PV_AVI_FILE_PARSER_SEEK_ERROR                = 20

}PV_AVI_FILE_PARSER_ERROR_TYPE;


//stores file parser status
class PVAviFileParserStatus
{

    public:
        PVAviFileParserStatus()
        {
            iLogger = PVLogger::GetLoggerObject("PVAviFileParser");
        }

        ~PVAviFileParserStatus()
        {
            iLogger = NULL;
        }

        PV_AVI_FILE_PARSER_ERROR_TYPE
        GetStatus()
        {
            if (iError != PV_AVI_FILE_PARSER_SUCCESS)
            {
                //Log error.
                PVAVIFILE_LOGERROR((0, "Error: %d occurred while parsing", iError));
                return iError;
            }
            else
            {
                PVAVIFILE_LOGINFO((0, "SUCCESS"));
                return iError;
            }
        }

    protected:

        PV_AVI_FILE_PARSER_ERROR_TYPE   iError;
        PVLogger*						iLogger;

};

#endif	//#ifndef PV_AVIFILE_STATUS

