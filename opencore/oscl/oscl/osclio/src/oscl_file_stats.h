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
// -*- c++ -*-
// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

//                     O S C L _ F I L E  _ STATS

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

/*! \addtogroup osclio OSCL IO
 *
 * @{
 */


/*! \file oscl_file_stats.h
    \brief File stats class

*/

#ifndef OSCL_FILE_STATS_H_INCLUDED
#define OSCL_FILE_STATS_H_INCLUDED

#include "oscl_base.h"
#include "osclconfig_io.h"

#define OSCL_FILE_STATS_LOGGER_NODE "OsclFileStats"

class OsclFileStatsItem
{
    public:
        uint32 iOpCount;
        uint32 iParam;
        TOsclFileOffset iParam2;
        uint32 iStartTick;
        uint32 iTotalTicks;
};

enum TOsclFileOp
{
    EOsclFileOp_Open
    , EOsclFileOp_Close
    , EOsclFileOp_Read
    , EOsclFileOp_Write
    , EOsclFileOp_Seek
    , EOsclFileOp_Tell
    , EOsclFileOp_Size
    , EOsclFileOp_Flush
    , EOsclFileOp_EndOfFile
    , EOsclFileOp_NativeOpen
    , EOsclFileOp_NativeClose
    , EOsclFileOp_NativeRead
    , EOsclFileOp_NativeWrite
    , EOsclFileOp_NativeSeek
    , EOsclFileOp_NativeTell
    , EOsclFileOp_NativeSize
    , EOsclFileOp_NativeFlush
    , EOsclFileOp_NativeEndOfFile
    , EOsclFileOp_Last
};
static const char* const TOsclFileOpStr[] =
{
    "Open"
    , "Close"
    , "Read"
    , "Write"
    , "Seek"
    , "Tell"
    , "Size"
    , "Flush"
    , "EndOfFile"
    , "NativeOpen"
    , "NativeClose"
    , "NativeRead"
    , "NativeWrite"
    , "NativeSeek"
    , "NativeTell"
    , "NativeSize"
    , "NativeFlush"
    , "NativeEndOfFile"
    , "???"
};

class PVLogger;
class Oscl_File;

class OsclFileStats
{
    public:
        OsclFileStats(Oscl_File* c);
        void Start(uint32& aTicks);
        void End(TOsclFileOp aOp, uint32 aStart, uint32 aParam = 0, TOsclFileOffset aParam2 = 0);
        void Log(TOsclFileOp, PVLogger*, uint32);
        void LogAll(PVLogger*, uint32);

    private:
        Oscl_File* iContainer;
        //fixed array of the max time per item.
        OsclFileStatsItem iStats[EOsclFileOp_Last];
};



#endif

/*! @} */

