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
// ----------------------------------------------------------------------
//
// This Software is an original work of authorship of PacketVideo Corporation.
// Portions of the Software were developed in collaboration with NTT  DoCoMo,
// Inc. or were derived from the public domain or materials licensed from
// third parties.  Title and ownership, including all intellectual property
// rights in and to the Software shall remain with PacketVideo Corporation
// and NTT DoCoMo, Inc.
//
// -----------------------------------------------------------------------
/************************************************************************/
/*  file name       : selcblccmn.h                                      */
/*  file contents   : Uni-Directional and Bi-Directional                */
/*                  :              Logical Channel Signalling Entity    */
/*                  :                          Common Management Header */
/*  draw            : '96.11.11                                         */
/*----------------------------------------------------------------------*/
/*  amendment       :                                                   */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
#ifndef _LCENTRY_H_
#define _LCENTRY_H_

#include <stdio.h>
#include "oscl_vector.h"
#include "oscl_mem.h"
#include "oscl_base.h"

/************************************************************************/
/*  Uni-Directional and Bi-Directional Logical Status Definition        */
/************************************************************************/
#define     LCBLC_OUTGOING_RLSED            0X00    /* Outgoing Released */
#define     LCBLC_OUTGOING_AWTING_ETBMNT    0X10    /* Outgoing Awaiting Establishement */
#define     LCBLC_OUTGOING_ETBED            0X20    /* Outgoing Established */
#define     LCBLC_OUTGOING_AWTING_RLS       0X30    /* Outgoing Awaiting Release */
#define     LCBLC_INCOMING_RLSED            0X01    /* Incoming Released */
#define     LCBLC_INCOMING_AWTING_ETBMNT    0X11    /* Incoming Awaiting Establishment */
#define     LCBLC_INCOMING_AWTING_CNFMTN    0X21    /* Incoming Awaiting Confirmation ( Bi-Directional Only ) */
#define     LCBLC_INCOMING_ETBED            0X31    /* Incoming Established */


/************************************************************************/
/*  Directional Information Definition                                  */
/************************************************************************/
#define     NO_DIRECTIONAL      0   /* No-Directional */
#define     UNI_DIRECTIONAL     1   /* Uni-Directional */
#define     BI_DIRECTIONAL      2   /* Bi-Directional */


/************************************************************************/
/*  CLC Definition                                                      */
/************************************************************************/
#define     CLS_OUTGOING_IDLE           0X00    /* Outgoing Idle */
#define     CLS_OUTGOING_AWTING_RPS     0X10    /* Outgoing Awaiting Response */
#define     CLS_INCOMING_IDLE           0X01    /* Incoming Idle */
#define     CLS_INCOMING_AWTING_RPS     0X11    /* Incoming Awaiting Response */


/************************************************************************/
/*  Class Definitions                                                   */
/************************************************************************/
typedef enum { LC_OUTGOING, LC_INCOMING } LCDirection_t;

class LCEntry
{
    public:
        LCEntry(uint32 lcn, LCDirection_t direction)
        {
            FwdLcn = lcn ;
            RvsLcn = 0 ;

            /********************************************************/
            /* NOTE: OUTGOING with respect to LCBLC                 */
            /********************************************************/
            if (direction == LC_OUTGOING)
            {
                Status = LCBLC_OUTGOING_RLSED ;
                ClcStatus = CLS_INCOMING_IDLE;
            }
            else
            {
                Status = LCBLC_INCOMING_RLSED ;
                ClcStatus = CLS_OUTGOING_IDLE;
            }

            Directional = NO_DIRECTIONAL ;
            TmrSqcNumber = 0 ;
            ClcTmrSqcNumber = 0;
        }
        ~LCEntry() {}

        void Reset()
        {
            RvsLcn = 0 ;

            if (Status & 0x01)    //LC_INCOMING
            {
                Status = LCBLC_INCOMING_RLSED ;
                ClcStatus = CLS_OUTGOING_IDLE;
            }
            else   //LC_OUTGOING
            {
                Status = LCBLC_OUTGOING_RLSED ;
                ClcStatus = CLS_INCOMING_IDLE;
            }

            Directional = NO_DIRECTIONAL ;
            TmrSqcNumber = 0 ;
            ClcTmrSqcNumber = 0;
        }

        uint32 FwdLcnRead()
        {
            return FwdLcn;
        }
        void FwdLcnWrite(uint32 fwdLcn)
        {
            FwdLcn = fwdLcn;
        }

        uint32 RvsLcnRead()
        {
            return RvsLcn;
        }
        void RvsLcnWrite(uint32 rvsLcn)
        {
            RvsLcn = rvsLcn;
        }

        uint8 StatusRead()
        {
            return Status;
        }
        void StatusWrite(uint8 status)
        {
#ifdef PVANALYZER
            StatusShow(status, Status);
#endif
            Status = status;

            // if LC is set to released state, then reset it's attributes so it can be used again
            if (Status == LCBLC_OUTGOING_RLSED || Status == LCBLC_INCOMING_RLSED)
            {
                RvsLcn = 0;
                Directional = NO_DIRECTIONAL ;
                TmrSqcNumber = 0 ;
            }
        }

        uint8 DirectionalRead()
        {
            return Directional;
        }
        void DirectionalWrite(uint8 directional)
        {
            Directional = directional;
        }

        void TmrSqcNumberClear()
        {
            TmrSqcNumber = 0;
        }
        int32 TmrSqcNumberRead()
        {
            return TmrSqcNumber;
        }
        int32 TmrSqcNumberInc()
        {
            return (++TmrSqcNumber);
        }

        uint8 ClcStatusRead()
        {
            return ClcStatus;
        }
        void ClcStatusWrite(uint8 status)
        {
#ifdef PVANALYZER
            ClcStatusShow(status, ClcStatus);
#endif
            ClcStatus = status;
        }

        void ClcTmrSqcNumberClear()
        {
            ClcTmrSqcNumber = 0;
        }
        int32 ClcTmrSqcNumberRead()
        {
            return ClcTmrSqcNumber;
        }
        void ClcTmrSqcNumberInc()
        {
            ClcTmrSqcNumber++;
        }

#ifdef PVANALYZER /* --------SE Analyzer Tool -------- */
#define ANALYZER_SE 0x0020		// (Assume tag is fixed)
        void StatusShow(uint8 oldStatus, uint8 newStatus)
        {
            void Show245(uint16 tag, uint16 indent, char* inString);
            char tempString[80];

            Show245(ANALYZER_SE, 0, "LCSE/BLCSE State Transition:");
            sprintf(tempString, "  LogicalChannelNumber = %u", FwdLcn);
            Show245(ANALYZER_SE, 0, tempString);
            sprintf(tempString, "  from--> %s", StateLabel(oldStatus));
            Show245(ANALYZER_SE, 0, tempString);
            sprintf(tempString, "    to--> %s", StateLabel(newStatus));
            Show245(ANALYZER_SE, 0, tempString);
            Show245(ANALYZER_SE, 0, " ");
        }

        char* StateLabel(uint8 status)
        {
            switch (status)
            {
                case LCBLC_OUTGOING_RLSED:
                    return("Outgoing RELEASED");
                    break;
                case LCBLC_OUTGOING_AWTING_ETBMNT:
                    return("Outgoing AWAITING ESTABLISHMENT");
                    break;
                case LCBLC_OUTGOING_ETBED:
                    return("Outgoing ESTABLISHED");
                    break;
                case LCBLC_OUTGOING_AWTING_RLS:
                    return("Outgoing AWAITING RELEASE");
                    break;
                case LCBLC_INCOMING_RLSED:
                    return("Incoming RELEASED");
                    break;
                case LCBLC_INCOMING_AWTING_ETBMNT:
                    return("Incoming AWAITING ESTABLISHMENT");
                    break;
                case LCBLC_INCOMING_AWTING_CNFMTN:
                    return("Incoming AWAITING CONFIRMATION");
                    break;
                case LCBLC_INCOMING_ETBED:
                    return("Incoming ESTABLISHED");
                    break;
                default:
                    return("UNKNOWN STATE");
            }
        }

        void ClcStatusShow(uint8 oldStatus, uint8 newStatus)
        {
            void Show245(uint16 tag, uint16 indent, char* inString);
            char tempString[80];

            Show245(ANALYZER_SE, 0, "CLCSE State Transition:");
            sprintf(tempString, "  LogicalChannelNumber = %u", FwdLcn);
            Show245(ANALYZER_SE, 0, tempString);
            sprintf(tempString, "  from--> %s", ClcStateLabel(oldStatus));
            Show245(ANALYZER_SE, 0, tempString);
            sprintf(tempString, "    to--> %s", ClcStateLabel(newStatus));
            Show245(ANALYZER_SE, 0, tempString);
            Show245(ANALYZER_SE, 0, " ");
        }

        char *ClcStateLabel(uint8 status)
        {
            switch (status)
            {
                case CLS_OUTGOING_IDLE:
                    return("Outgoing IDLE");
                    break;
                case CLS_OUTGOING_AWTING_RPS:
                    return("Outgoing AWAITING RESPONSE");
                    break;
                case CLS_INCOMING_IDLE:
                    return("Incoming IDLE");
                    break;
                case CLS_INCOMING_AWTING_RPS:
                    return("Incoming AWAITING RESPONSE");
                    break;
                default:
                    return("UNKNOWN STATE");
            }
        }

#endif            /* --------------------------------- */

    private:
        LCEntry(const LCEntry&);

        uint32    FwdLcn ;
        uint32    RvsLcn ;
        uint8   Directional ;
        uint8   Status ;
        int32     TmrSqcNumber ;
        uint8   ClcStatus;          /* Multiple CLCSEs (RAN) */
        int32     ClcTmrSqcNumber;    /* Multiple CLCSEs (RAN) */
};

class LCEntryList
{
    public:
        LCEntryList() : Direction(LC_OUTGOING) {}
        LCEntryList(LCDirection_t direction) : Direction(direction) {}

        ~LCEntryList()
        {
            // delete all LCEntry's remaining
            for (Oscl_Vector<LCEntry*, OsclMemAllocator>::iterator it = EntryList.begin(); it != EntryList.end(); it++) OSCL_DELETE((*it));
        }

        void Reset()
        {
            // reset all LCEntry's remaining
            for (Oscl_Vector<LCEntry*, OsclMemAllocator>::iterator it = EntryList.begin(); it != EntryList.end(); it++)(*it)->Reset();
        }

        void SetDirection(LCDirection_t direction)
        {
            Direction = direction;
        }

        LCEntry* NewLCEntry(uint32 lcn)
        {
            // allocate a new LCEntry
            LCEntry *lcentry = OSCL_NEW(LCEntry, (lcn, Direction));
            OsclError::LeaveIfNull(lcentry);
            // add it to the list
            EntryList.push_back(lcentry);

            return lcentry;
        }

        LCEntry* GetLCEntry(uint32 lcn)
        {
            lcn = (lcn & 0xFFFF);
            // look for existing LCEntry
            for (Oscl_Vector<LCEntry*, OsclMemAllocator>::iterator it = EntryList.begin(); it != EntryList.end(); it++)
            {
                if ((*it)->FwdLcnRead() == lcn)
                {
                    return (*it);
                }
            }

            return NULL;
        }

        void DeleteLCEntry(uint32 lcn)
        {
            for (Oscl_Vector<LCEntry*, OsclMemAllocator>::iterator it = EntryList.begin(); it != EntryList.end(); it++)
            {
                if ((*it)->FwdLcnRead() == lcn)
                {
                    OSCL_DELETE((*it));
                    EntryList.erase(it);
                    return;
                }
            }
        }

        void DeleteLCEntry(LCEntry *lcentry)
        {
            for (Oscl_Vector<LCEntry*, OsclMemAllocator>::iterator it = EntryList.begin(); it != EntryList.end(); it++)
            {
                if ((*it) == lcentry)
                {
                    OSCL_DELETE((*it));
                    EntryList.erase(it);
                    return;
                }
            }
        }

    private:
        LCDirection_t Direction;
        Oscl_Vector<LCEntry*, OsclMemAllocator> EntryList;
};

#endif /* _LCENTRY_H_ */
