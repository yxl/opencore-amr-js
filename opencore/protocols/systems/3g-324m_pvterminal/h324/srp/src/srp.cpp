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

#include "srp.h"
#include "srp_ports.h"
#include "pvlogger.h"

/************************************************************************/
/*  Constants			                                                */
/************************************************************************/
const SRP::SrpAction SRP::iSrpStateTable[MAX_STATUS][MAX_EVENT] =
{
    /* Event No.		 0						1					2				3				4				5					6				7		*/
    /* Status 0 */ { SRP::ACTION_NOP,  SRP::ACTION_0_1, SRP::ACTION_0_2, SRP::ACTION_NOP,  SRP::ACTION_NOP, SRP::ACTION_NOP, SRP::ACTION_0_2, SRP::ACTION_NOP} ,
    /* Status 1 */ { SRP::ACTION_NOP,  SRP::ACTION_1_1, SRP::ACTION_0_2, SRP::ACTION_1_3,  SRP::ACTION_1_3, SRP::ACTION_1_4, SRP::ACTION_0_2, SRP::ACTION_1_3}
};


OSCL_EXPORT_REF SRP::SRP() : iN400MaxCounter(N400_DEFAULT_MAX_COUNTER),  //changed max # of allowable retries from 10 to 200
        //This was done in conjunction with lowering the SRP
        //timeout value from 400ms to 50ms
        iN402MaxCounter(N402_DEFAULT_MAX_COUNTER), //WNSRP counter
        iCrcData(0),
        iSendSeqNumber(0),
        iRecvSeqNumber(0),
        iStatus(STS_IDLE),
        iCurWnsrpStatus(WNSRP_TX_SUPPORT),
        iT401TimerValue(T401_VALUE_DEFAULT),
        iWnsrpCommandSave(NULL),
        iCcsrlSduSize(DEFAULT_CCSRL_SIZE),
        iRxFrags(NULL),
        iFreeRxFragList(NULL),
        iPendingRxFragList(NULL),
        iFirstCmd(true),
        iLLPortOut(NULL),
        iLLPortIn(NULL),
        iULPortOut(NULL),
        iULPortIn(NULL),
        iTxMediaMsgPoolAlloc(NULL),
        iTxMediaDataImplMemAlloc(NULL),
        iTxPacketAlloc(NULL),
        iOutstandingTxMediaMsgPoolAlloc(NULL),
        iOutstandingTxMediaDataImplMemAlloc(NULL),
        iRxMediaMsgPoolAlloc(NULL),
        iRxMediaDataImplMemAlloc(NULL),
        iRxPacketAlloc(NULL),
        iRespMediaMsgPoolAlloc(NULL),
        iRespMemAlloc(NULL),
        iRespPacketAlloc(NULL),
        iNsrpRespMemAlloc(NULL),
        iNsrpRespPacketAlloc(NULL),
        iNumTxMsgs(DEFAULT_SEND_QUEUE_SIZE),
        iUseNSRP(true),
        iFirstNSRPResp(false),
        iHandleWNSRP(true),
        iWNSRPTxWindow(DEFAULT_WNSRP_WINDOW),
        iWNSRPRxWindow(DEFAULT_WNSRP_WINDOW),
        iOldestWNSRPRetransSeqNum(0),
        iT401Resolution(T401_RESOLUTION),
        iObserver(NULL)
{
    iLogger = PVLogger::GetLoggerObject("3g324m.h324.srp");
}

OSCL_EXPORT_REF SRP::~SRP()
{
}

OSCL_EXPORT_REF void SRP::SrpInitL(void)
{
    iLLPortOut = OSCL_NEW(SRPLowerLayerPortOut, ());
    OsclError::LeaveIfNull(iLLPortOut);
    iLLPortIn  = OSCL_NEW(SRPLowerLayerPortIn, (this));
    OsclError::LeaveIfNull(iLLPortIn);
    iULPortOut = OSCL_NEW(SRPUpperLayerPortOut, ());
    OsclError::LeaveIfNull(iULPortOut);
    iULPortIn  = OSCL_NEW(SRPUpperLayerPortIn, (this));
    OsclError::LeaveIfNull(iULPortIn);

    uint32 i;
    iSrpCommandSave.timerID = iWNSRPTxWindow;

    iWnsrpCommandSave = OSCL_ARRAY_NEW(SRPRespTimer, iWNSRPTxWindow);
    OsclError::LeaveIfNull(iWnsrpCommandSave);
    for (i = 0; i < iWNSRPTxWindow; i++)
    {
        iWnsrpCommandSave[i].timerID = i;
    }

    iRxFrags = OSCL_ARRAY_NEW(SRPRxData, iWNSRPRxWindow);
    OsclError::LeaveIfNull(iRxFrags);
    for (i = 0; i < iWNSRPRxWindow; i++)
    {
        FreeRxFrag(&iRxFrags[i]);
    }

    iTxMediaMsgPoolAlloc = OSCL_NEW(OsclMemPoolFixedChunkAllocator, (iNumTxMsgs, 0, &iMemAllocator));
    OsclError::LeaveIfNull(iTxMediaMsgPoolAlloc);
    iTxMediaDataImplMemAlloc = OSCL_NEW(OsclMemPoolFixedChunkAllocator, (iNumTxMsgs + ((iWNSRPTxWindow + 1) * 2), 0, &iMemAllocator));
    OsclError::LeaveIfNull(iTxMediaDataImplMemAlloc);
    iTxPacketAlloc = OSCL_NEW(PVMFMediaFragGroupCombinedAlloc<OsclMemAllocator>, (iNumTxMsgs + ((iWNSRPTxWindow + 1) * 2), MAX_SEND_FRAGS_PER_MSG, iTxMediaDataImplMemAlloc));
    OsclError::LeaveIfNull(iTxPacketAlloc);
    iTxPacketAlloc->create();

    iOutstandingTxMediaMsgPoolAlloc = OSCL_NEW(OsclMemPoolFixedChunkAllocator, ((iWNSRPTxWindow + 1) * 2, 0, &iMemAllocator));
    OsclError::LeaveIfNull(iOutstandingTxMediaMsgPoolAlloc);
    iOutstandingTxMediaDataImplMemAlloc = OSCL_NEW(OsclMemPoolFixedChunkAllocator, ((iWNSRPTxWindow + 1) * 2, 0, &iMemAllocator));
    OsclError::LeaveIfNull(iOutstandingTxMediaDataImplMemAlloc);

    iRxMediaMsgPoolAlloc = OSCL_NEW(OsclMemPoolFixedChunkAllocator, (iWNSRPRxWindow * 2, 0, &iMemAllocator));
    OsclError::LeaveIfNull(iRxMediaMsgPoolAlloc);
    iRxMediaDataImplMemAlloc = OSCL_NEW(OsclMemPoolFixedChunkAllocator, (iWNSRPRxWindow * 2, 0, &iMemAllocator));
    OsclError::LeaveIfNull(iRxMediaDataImplMemAlloc);
    iRxPacketAlloc = OSCL_NEW(PVMFSimpleMediaBufferCombinedAlloc, (iRxMediaDataImplMemAlloc));
    OsclError::LeaveIfNull(iRxPacketAlloc);

    iRespMediaMsgPoolAlloc = OSCL_NEW(OsclMemPoolFixedChunkAllocator, (MAX_RESP_MSGS, 0, &iMemAllocator));
    OsclError::LeaveIfNull(iRespMediaMsgPoolAlloc);
    iRespMemAlloc = OSCL_NEW(OsclMemPoolFixedChunkAllocator, (MAX_RESP_MSGS, 0, &iMemAllocator));
    OsclError::LeaveIfNull(iRespMemAlloc);
    iRespPacketAlloc = OSCL_NEW(PVMFSimpleMediaBufferCombinedAlloc, (iRespMemAlloc));
    OsclError::LeaveIfNull(iRespPacketAlloc);
    iNsrpRespMemAlloc = OSCL_NEW(OsclMemPoolFixedChunkAllocator, (MAX_RESP_MSGS, 0, &iMemAllocator));
    OsclError::LeaveIfNull(iNsrpRespMemAlloc);
    iNsrpRespPacketAlloc = OSCL_NEW(PVMFSimpleMediaBufferCombinedAlloc, (iNsrpRespMemAlloc));
    OsclError::LeaveIfNull(iNsrpRespPacketAlloc);

    iHdrAllocator.size((uint16) iNumTxMsgs*2, (SRP_HEADER_SIZE));
    iHdrAllocator.SetLeaveOnAllocFailure(false);
    iCcsrlAllocator.size((uint16) iNumTxMsgs*2, CCSRL_HEADER_SIZE + SRP_SEQUENCE_SIZE);
    iCcsrlAllocator.SetLeaveOnAllocFailure(false);
    iFCSAllocator.size((uint16) iNumTxMsgs*2, SRP_FCS_SIZE);
    iFCSAllocator.SetLeaveOnAllocFailure(false);

    //Just need 1 fragment for SRP/NSRP messages and 1 for WNSRP messages
    iMTEntryNumAllocator.size(NUM_MT_ENTRY_NUMBERS, MT_ENTRY_NUMBER_SIZE);
    iMTEntryNumAllocator.SetLeaveOnAllocFailure(false);
    iSrpNsrpEntryNumFrag = iMTEntryNumAllocator.get();
    *(uint8 *)iSrpNsrpEntryNumFrag.getMemFragPtr() = SRP_NSRP_MT_ENTRY_NUMBER;
    iWnsrpEntryNumFrag = iMTEntryNumAllocator.get();
    *(uint8 *)iWnsrpEntryNumFrag.getMemFragPtr() = WNSRP_MT_ENTRY_NUMBER;

    iSrpWaitQueue.reserve(iNumTxMsgs);

    iFreeWnsrpCommandSave.reserve(iWNSRPTxWindow);
    iActiveWnsrpCommandSave.reserve(iWNSRPTxWindow);

    iSrpCommandSave.T401Timer.SetFrequency(T401_RESOLUTION);
    for (i = 0; i < iWNSRPTxWindow; i++)
    {
        iWnsrpCommandSave[i].T401Timer.SetFrequency(T401_RESOLUTION);
        iFreeWnsrpCommandSave.push_back(&iWnsrpCommandSave[i]);
    }

    //Temporary for now, should be called when tsc does a connect.
    SrpStart();
}

OSCL_EXPORT_REF void SRP::SrpReset(void)
{
    //Temporary for now, should be called when tsc does a disconnect.
    SrpStop();

    OsclRefCounterMemFrag temp;
    iSrpNsrpEntryNumFrag = temp;
    iWnsrpEntryNumFrag = temp;

    iActiveWnsrpCommandSave.clear();
    iFreeWnsrpCommandSave.clear();

    iMTEntryNumAllocator.clear();
    iHdrAllocator.clear();
    iCcsrlAllocator.clear();
    iFCSAllocator.clear();

    if (iTxPacketAlloc)
    {
        iTxPacketAlloc->removeRef();
        iTxPacketAlloc = NULL;
    }

    if (iTxMediaDataImplMemAlloc)
    {
        OSCL_DELETE(iTxMediaDataImplMemAlloc);
        iTxMediaDataImplMemAlloc = NULL;
    }

    if (iTxMediaMsgPoolAlloc)
    {
        OSCL_DELETE(iTxMediaMsgPoolAlloc);
        iTxMediaMsgPoolAlloc = NULL;
    }

    if (iOutstandingTxMediaDataImplMemAlloc)
    {
        OSCL_DELETE(iOutstandingTxMediaDataImplMemAlloc);
        iOutstandingTxMediaDataImplMemAlloc = NULL;
    }

    if (iOutstandingTxMediaMsgPoolAlloc)
    {
        OSCL_DELETE(iOutstandingTxMediaMsgPoolAlloc);
        iOutstandingTxMediaMsgPoolAlloc = NULL;
    }

    if (iRxPacketAlloc)
    {
        OSCL_DELETE(iRxPacketAlloc);
        iRxPacketAlloc = NULL;
    }

    if (iRxMediaDataImplMemAlloc)
    {
        OSCL_DELETE(iRxMediaDataImplMemAlloc);
        iRxMediaDataImplMemAlloc = NULL;
    }

    if (iRxMediaMsgPoolAlloc)
    {
        OSCL_DELETE(iRxMediaMsgPoolAlloc);
        iRxMediaMsgPoolAlloc = NULL;
    }

    if (iNsrpRespMemAlloc)
    {
        OSCL_DELETE(iNsrpRespMemAlloc);
        iNsrpRespMemAlloc = NULL;
    }

    if (iNsrpRespPacketAlloc)
    {
        OSCL_DELETE(iNsrpRespPacketAlloc);
        iNsrpRespPacketAlloc = NULL;
    }

    if (iRespMemAlloc)
    {
        OSCL_DELETE(iRespMemAlloc);
        iRespMemAlloc = NULL;
    }

    if (iRespPacketAlloc)
    {
        OSCL_DELETE(iRespPacketAlloc);
        iRespPacketAlloc = NULL;
    }

    if (iRespMediaMsgPoolAlloc)
    {
        OSCL_DELETE(iRespMediaMsgPoolAlloc);
        iRespMediaMsgPoolAlloc = NULL;
    }

    if (iWnsrpCommandSave)
    {
        OSCL_ARRAY_DELETE(iWnsrpCommandSave);
        iWnsrpCommandSave = NULL;
    }

    if (iRxFrags)
    {
        OSCL_ARRAY_DELETE(iRxFrags);
        iRxFrags = NULL;
    }

    if (iLLPortOut)
    {
        OSCL_DELETE(iLLPortOut);
        iLLPortOut = NULL;
    }

    if (iLLPortIn)
    {
        OSCL_DELETE(iLLPortIn);
        iLLPortIn = NULL;
    }

    if (iULPortOut)
    {
        OSCL_DELETE(iULPortOut);
        iULPortOut = NULL;
    }

    if (iULPortIn)
    {
        OSCL_DELETE(iULPortIn);
        iULPortIn = NULL;
    }

    WnsrpStatusSet(WNSRP_TX_SUPPORT);
    iHandleWNSRP = true;
}


OSCL_EXPORT_REF void SRP::SrpStart(void)
{
    SrpResetStats();

    iFirstCmd = true;

    /* Clear receive sequence number */
    SrpRecvSeqClear();
    /* Clear send sequence number */
    SrpSendSeqClear();

    /* Clear WNSRP retry counter */
    SrpN402Initialize();
    /* Set status */
    SrpStatusSet(STS_IDLE);
}

OSCL_EXPORT_REF void SRP::SrpStop(void)
{
    SRPRxData *pendingRxFrag;

    // Cancel any outstanding timer events
    if (iSrpCommandSave.isTimerActive)
    {
        SrpT401Stop(iSrpCommandSave);
        iSrpCommandSave.pPkt.Unbind();
    }

    //Clear all WNSRP resp timers
    while (!iActiveWnsrpCommandSave.empty())
    {
        // Stop response wait timer
        SrpT401Stop(*iActiveWnsrpCommandSave[0]);

        // Release packet
        iActiveWnsrpCommandSave[0]->pPkt.Unbind();

        // Remove timer from active list
        RemoveActiveWnsrpTimer(iActiveWnsrpCommandSave[0]);
    }

    WnsrpStatusSet(WNSRP_TX_SUPPORT);

    // Flush the queue
    iSrpWaitQueue.clear();

    // Clear pending received packet.
    while (iPendingRxFragList)
    {
        pendingRxFrag = iPendingRxFragList;
        iPendingRxFragList = iPendingRxFragList->next;
        FreeRxFrag(pendingRxFrag);
    }


    /* AR: Switch back to SRP mode */
    iUseNSRP = false;

    /* Print iStats */
    SRPStats tempStats;
    SrpGetStats(tempStats);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STATISTIC, (0x40000020, "SRP Stats:"));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STATISTIC, (0x40000020, "Total frames recv %d", tempStats.totalFramesRecv));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STATISTIC, (0x40000020, "Invalid size frames recv %d", tempStats.totalInvalidSizeFramesRecv));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STATISTIC, (0x40000020, "CRC errors recv %d", tempStats.totalCRCErrorsRecv));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STATISTIC, (0x40000020, "Cmd frames recv %d", tempStats.totalCommandFramesRecv));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STATISTIC, (0x40000020, "Cmd WNSRP frames recv %d", tempStats.totalWNSRPCommandFramesRecv));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STATISTIC, (0x40000020, "Invalid CCSRL chunk recv %d", tempStats.totalInvalidCCSRLChunkRecv));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STATISTIC, (0x40000020, "SRP resp recv %d", tempStats.totalSRPRespRecv));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STATISTIC, (0x40000020, "NSRP resp recv %d", tempStats.totalNSRPRespRecv));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STATISTIC, (0x40000020, "WNSRP resp recv %d", tempStats.totalWNSRPRespRecv));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STATISTIC, (0x40000020, "H245 messages recv %d", tempStats.totalH245MessagesRecv));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STATISTIC, (0x40000020, "CCSRL chunks recv %d", tempStats.totalCCSRLChunksRecv));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STATISTIC, (0x40000020, "Cmd frames sent %d", tempStats.totalCommandFramesSent));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STATISTIC, (0x40000020, "H245 messages to send %d", tempStats.totalH245MessagesToSend));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STATISTIC, (0x40000020, "H245 messages fragmented %d", tempStats.totalH245MessagesFragmented));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STATISTIC, (0x40000020, "CCSRL chunks sent %d", tempStats.totalCCSRLChunksSent));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STATISTIC, (0x40000020, "Num of timeouts %d", tempStats.totalNumTimeouts));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STATISTIC, (0x40000020, "Bytes recv %d", tempStats.totalBytesRecv));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STATISTIC, (0x40000020, "Bytes sent %d", tempStats.totalBytesSent));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STATISTIC, (0x40000020, "Invalid frames %d", tempStats.totalInvalidFrames));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STATISTIC, (0x40000020, "Min response time %d", tempStats.minRespTime));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STATISTIC, (0x40000020, "Max response time %d", tempStats.maxRespTime));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STATISTIC, (0x40000020, "Ave response time %d", tempStats.aveRespTime));
}

/************************************************************************/
/*  function name       : SrpGetEventNoFromAL1                          */
/*  function outline    : decide event number from received PDU         */
/*                                                          (H.223 AL1) */
/*  function discription: INT SrpGetEventNoFromAL1(						*/
/*									PVMFSharedMediaDataPtr pPkt )		*/
/*  input data          : pPkt - pointer of received packet             */
/*  output data         : event number                                  */
/*  draw time           : '96.10.29                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
SRP::SrpEvent SRP::SrpGetEventNoFromAL1(PVMFSharedMediaDataPtr pPkt)
{
    SrpEvent EventNo = EV_NOP;
    uint8 *pFrame;
    uint8 CrcWork[2];
    OsclRefCounterMemFrag frag;
    OsclSharedPtr<PVMFMediaDataImpl> data;
    uint16 Crc16Check(PVMFSharedMediaDataPtr aData);

    if (pPkt->getNumFragments() != 1)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_VERBOSE, (0x40000020, "SRP:SrpGetEventNoFromAL1 - fragments not equal to 1, num frags %d", pPkt->getNumFragments()));
        return EventNo;
    }

    if (pPkt->getFilledSize() >= MINIMUM_FRAME_SIZE)
    {
        pPkt->getMediaDataImpl(data);

        CrcClear();
        CrcSetToFrame(iCrc.Crc16Check(data, true), &CrcWork[0]);

        pPkt->getMediaFragment(0, frag);
        pFrame = (uint8 *) frag.getMemFragPtr();

        if ((CrcWork[0] == pFrame[pPkt->getFilledSize()-SRP_FCS_SIZE]) &&
                (CrcWork[1] == pFrame[pPkt->getFilledSize()-SRP_FCS_SIZE+1]))
        {
            switch (pFrame[0])
            {
                case SRP_COMMAND_HEADER:
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0x40000020, "SRP:SrpGetEventNoFromAL1 - SRP_COMMAND_HEADER, seq num %d", pFrame[1]));

                    //Disable WNSRP if 1st command frame does not start with seq num 0
                    if ((WnsrpStatusGet() == WNSRP_TX_SUPPORT) &&
                            iFirstCmd &&
                            (pFrame[1] != 0))
                    {
                        WnsrpStatusSet(NO_WNSRP_SUPPORT);
                    }

                    iStats.totalCommandFramesRecv++;

                    pPkt->setSeqNum((uint32) pFrame[1]);

                    //If using CCSRL method.
                    if (iCcsrlSduSize > 0)
                    {
                        //Check if is a valid CCSRL chunk.
                        if ((pFrame[2] == INTERMEDIATE_CCSRL_CHUNK) || (pFrame[2] == LAST_CCSRL_CHUNK))
                        {
                            EventNo = EV_COMMAND;
                        }
                        else
                        {
                            // Record invalid CCSRL chunk
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0x40000020, "SRP: Invalid CCSRL Chunk"));
                            iStats.totalInvalidCCSRLChunkRecv++;
                        }
                    }
                    //Else not using CCSRL method
                    else
                    {
                        EventNo = EV_COMMAND;
                    }

                    break;

                case SRP_RESPONSE_HEADER: // No sequence number in SRP responses
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0x40000020, "SRP:SrpGetEventNoFromAL1 - SRP_RESPONSE_HEADER"));

                    if (WnsrpStatusGet() == WNSRP_TX_SUPPORT)
                    {
                        SrpN402Count();
                        if (SrpN402Check() == false)
                        {
                            WnsrpStatusSet(NO_WNSRP_SUPPORT);
                        }
                    }

                    iStats.totalSRPRespRecv++;

                    // If using NSRP
                    if (iUseNSRP)
                    {
                        // If we haven't received an NSRP response yet, keep accepting SRP responses.
                        if (!iFirstNSRPResp)
                        {
                            EventNo = EV_RESPONSE;
                        }
                    }
                    // Else not using NSRP
                    else
                    {
                        EventNo = EV_RESPONSE;
                    }
                    break;

                case NSRP_RESPONSE_HEADER:
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0x40000020, "SRP:SrpGetEventNoFromAL1 - NSRP_RESPONSE_HEADER, seq num %d", pFrame[1]));

                    if (WnsrpStatusGet() == WNSRP_TX_SUPPORT)
                    {
                        SrpN402Count();
                        if (SrpN402Check() == false)
                        {
                            WnsrpStatusSet(NO_WNSRP_SUPPORT);
                        }
                    }

                    iStats.totalNSRPRespRecv++;

                    // If using NSRP, check sequence number.
                    if (iUseNSRP)
                    {
                        pPkt->setSeqNum((uint32) pFrame[1]);

                        if (!iFirstNSRPResp)
                        {
                            iFirstNSRPResp = true;
                        }

                        EventNo = EV_NSRP_RESPONSE;
                    }
                    //Else using SRP, just accept as a valid SRP response frame
                    else
                    {
                        EventNo = EV_RESPONSE;
                    }
                    break;

                case WNSRP_COMMAND_HEADER:
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0x40000020, "SRP:SrpGetEventNoFromAL1 - WNSRP_COMMAND_HEADER, status %d, seq num %d", WnsrpStatusGet(), pFrame[1]));

                    if (!iHandleWNSRP)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING, (0x40000020, "SRP:SrpGetEventNoFromAL1 - WNSRP_COMMAND_HEADER, not handling WNSRP frames"));
                        break;
                    }

                    iStats.totalWNSRPCommandFramesRecv++;

                    pPkt->setSeqNum((uint32) pFrame[1]);

                    switch (WnsrpStatusGet())
                    {
                        case WNSRP_TX_SUPPORT:
                            WnsrpStatusSet(WNSRP_FULL_SUPPORT);
                            //Fall into next case.

                        case WNSRP_FULL_SUPPORT:
                            //If using CCSRL method.
                            if (iCcsrlSduSize > 0)
                            {
                                //Check if is a valid CCSRL chunk.
                                if ((pFrame[2] == INTERMEDIATE_CCSRL_CHUNK) || (pFrame[2] == LAST_CCSRL_CHUNK))
                                {
                                    EventNo = EV_WNSRP_COMMAND;
                                }
                                else
                                {
                                    // Record invalid CCSRL chunk
                                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0x40000020, "SRP: Invalid CCSRL Chunk"));
                                    iStats.totalInvalidCCSRLChunkRecv++;
                                }
                            }
                            //Else not using CCSRL method
                            else
                            {
                                EventNo = EV_WNSRP_COMMAND;
                            }

                            break;

                        case NO_WNSRP_SUPPORT:
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING, (0x40000020, "SRP:SrpGetEventNoFromAL1 - no WNSRP support"));
                            break;
                    }

                    break;

                case WNSRP_RESPONSE_HEADER:
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0x40000020, "SRP:SrpGetEventNoFromAL1 - WNSRP_RESPONSE_HEADER, status %d, seq num %d", WnsrpStatusGet(), pFrame[1]));

                    if (!iHandleWNSRP)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING, (0x40000020, "SRP:SrpGetEventNoFromAL1 - WNSRP_COMMAND_HEADER, not handling WNSRP frames"));
                        break;
                    }

                    iStats.totalWNSRPRespRecv++;

                    switch (WnsrpStatusGet())
                    {
                        case WNSRP_TX_SUPPORT:
                            WnsrpStatusSet(WNSRP_FULL_SUPPORT);
                            //Fall into next case.

                        case WNSRP_FULL_SUPPORT:
                            //Set sequence number of the response.
                            pPkt->setSeqNum((uint32) pFrame[1]);

                            EventNo = EV_WNSRP_RESPONSE;
                            break;

                        case NO_WNSRP_SUPPORT:
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING, (0x40000020, "SRP:SrpGetEventNoFromAL1 - no WNSRP support"));
                            break;
                    }
                    break;

                default:
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0x40000020, "SRP: Unknown SRP command %d", pFrame[0]));
                    iStats.totalInvalidFrames++;
                    break;
            }
        }
        // CRC errors.
        else
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0x40000020, "SRP: CRC check failed"));
            iStats.totalCRCErrorsRecv++;
        }
    }
    else
    {
        // Record number of frames received that have an invalid size.
        iStats.totalInvalidSizeFramesRecv++;
    }

    return(EventNo) ;
}

/************************************************************************/
/*  function name       : SrpStateChange		                        */
/*  function outline    : decide function to call given the status and  */
/*                        event											*/
/*  function discription: INT SrpGetEventNoFromAL1( void *data )		*/
/*  input data          : status - Status of Srp		                */
/*						  event - Incoming event						*/
/*						  data - optional state change data				*/
/*  output data         : None			                                */
/*  draw time           : '96.10.29                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
int SRP::SrpStateChange(int status, int event, void *data)
{
    switch (iSrpStateTable[status][event])
    {
        case ACTION_NOP:
            return (SrpNop());
        case ACTION_0_1:
            return (Srp_0_1());
        case ACTION_0_2:
            return (Srp_0_2(*(PVMFSharedMediaDataPtr *) data, event));


        case ACTION_1_1:
            return (Srp_1_1());
        case ACTION_1_3:
            return (Srp_1_3(*(PVMFSharedMediaDataPtr *) data, event));
        case ACTION_1_4:
            return (Srp_1_4(*(int32 *) data));
        default:
            return 0;
    }
}

/************************************************************************/
/*  function name       : SrpN400Check                                  */
/*  function outline    : N400 counter check                            */
/*  function discription: INT  SrpN400Check(SRPRespTimer &timer)        */
/*  input data          : timer - timer to check                        */
/*  output data         : Check result                                  */
/*                         OK : Not count over                          */
/*                         NG : Count over                              */
/*  draw time           : '96.10.29                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
int SRP::SrpN400Check(SRPRespTimer &timer)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_VERBOSE, (0x40000020, "SRP::SrpN400Check - N400Counter=%d N400MaxCounter=%d", timer.N400Counter, iN400MaxCounter));

    if (timer.N400Counter < iN400MaxCounter)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/************************************************************************/
/*  function name       : SrpN402Initialize                             */
/*  function outline    : N402 counter initialize                       */
/*  function discription: void SrpN402Initialize( void )                */
/*  input data          : None                                          */
/*  output data         : None                                          */
/*  draw time           : '05.08.23                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                          Copyright (C) 2005 PacketVideo Corp.        */
/************************************************************************/
void SRP::SrpN402Initialize()
{
    /* Clear counter */
    iN402Counter = 0 ;
}


/************************************************************************/
/*  function name       : SrpN402Check                                  */
/*  function outline    : N402 counter check                            */
/*  function discription: INT  SrpN402Check( void )                     */
/*  input data          : None                                          */
/*  output data         : Check result                                  */
/*                         OK : Not count over                          */
/*                         NG : Count over                              */
/*  draw time           : '05.08.23                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                          Copyright (C) 2005 PacketVideo Corp.        */
/************************************************************************/
int SRP::SrpN402Check()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_VERBOSE, (0x40000020, "SRP::SrpN402Check - N402Counter=%d N402MaxCounter=%d", iN402Counter, iN402MaxCounter));

    if (iN402Counter < iN402MaxCounter)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/************************************************************************/
/*  function name       : CrcCalc                                       */
/*  function outline    : CRC calculation                               */
/*  function discription: void CrcCalc( PUCHAR pData, int Size )        */
/*  input data          : pData - object data pointer                   */
/*                        Size  - object data size (octet length)       */
/*  output data         : None                                          */
/*  draw time           : '96.10.29                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void SRP::CrcCalc(uint8 *pData, int Size)
{
    int     BitCount;

    while (Size--)  /* process for all data */
    {
        for (BitCount = 0 ; BitCount < 8 ; BitCount++)  /* process for 8 bit data */
        {
            if ((iCrcData & 0x8000) != 0)   /* MSB=1? */
            {
                iCrcData <<= 1;
                iCrcData |= ((*pData >> BitCount) & 0x01);
                iCrcData ^= GX_CRC_12;
            }
            else
            {
                iCrcData <<= 1;
                iCrcData |= ((*pData >> BitCount) & 0x01);
            }
        }

        /* update pointer of data */
        pData++;
    }
    return ;
}

/************************************************************************/
/*  function name       : CrcResultGet                                  */
/*  function outline    : Get result of CRC calculation                 */
/*  function discription: USHORT CrcResultGet( void )                   */
/*  input data          : None                                          */
/*  output data         : Result of CRC Calculation                     */
/*  draw time           : '96.10.29                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
uint16 SRP::CrcResultGet(void)
{
    int	BitCount;

    for (BitCount = 0 ; BitCount < 16 ; BitCount++)  /* process for 16bit */
    {
        if ((iCrcData & 0x8000) != 0)   /* MSB=1? */
        {
            iCrcData <<= 1;
            iCrcData  ^= GX_CRC_12;
        }
        else
        {
            iCrcData <<= 1;
        }
    }
    return((uint16)iCrcData) ;
}

/************************************************************************/
/*  function name       : CrcSetToFrame                                 */
/*  function outline    : Set CRC data to FCS area of frame             */
/*  function discription: void CrcSetToFrame( USHORT Crc, PUCHAR pFcs ) */
/*  input data          : None                                          */
/*  output data         : Crc - CRC data                                */
/*                        pFcs - FCS data pointer                       */
/*  draw time           : '96.10.29                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void SRP::CrcSetToFrame(uint16 crc, uint8 *pFcs)
{

    /* Clear first fcs octet */
    *pFcs = 0;

    *pFcs = (uint8)(crc & 0x00ff);
    pFcs++;
    *pFcs = (uint8)((crc & 0xff00) >> 8);

    return ;
}

/************************************************************************/
/*  function name       : SrpCommandCreate                              */
/*  function outline    : Create command frame                          */
/*  function discription: void SrpCommandCreate(                        */
/*                                    PVMFSharedMediaDataPtr pPkt )		*/
/*  input data          : pPkt - data packet to create SRP command frame*/
/*  output data         : None                                          */
/*  draw time           : '05.08.24                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                          Copyright (C) 2005 PacketVideo Corp.        */
/************************************************************************/
PVMFSharedMediaDataPtr SRP::SrpCommandCreate(PVMFSharedMediaDataPtr pPkt, uint8 header)
{
    uint16 crc;
    uint16 Crc16Check(PVMFSharedMediaDataPtr aData);
    int32 error = OsclErrNone;
    OsclSharedPtr<PVMFMediaDataImpl> srpPkt;
    PVMFSharedMediaDataPtr txData;
    OsclRefCounterMemFrag headerFrag;
    OsclRefCounterMemFrag FCSFrag;

    OSCL_UNUSED_ARG(error);
    switch (header)
    {
        case SRP_COMMAND_HEADER:
        case WNSRP_COMMAND_HEADER:
            srpPkt = iTxPacketAlloc->allocate();
            if (!srpPkt)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0x40000020, "SRP::SrpCommandCreate - Unable allocate tx media impl %d", error));
                return txData;
            }
            break;

        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0x40000020, "SRP::SrpCommandCreate - Unable command type %x", header));
            return txData;

    }

    // Allocate header fragment.
    headerFrag = iHdrAllocator.get();
    if (headerFrag.getMemFragPtr() == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0x40000020, "SRP::SrpCommandCreate Unable to allocate header frag %d", error));
        return txData;
    }


    // Allocate fcs fragment.
    FCSFrag = iFCSAllocator.get();
    if (FCSFrag.getMemFragPtr() == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0x40000020, "SRP::SrpCommandCreate Unable to allocate FCS frag %d", error));
        return txData;
    }


    //Set header value
    *(uint8 *) headerFrag.getMemFragPtr() = header;
    headerFrag.getMemFrag().len = SRP_HEADER_SIZE;

    // Add header to the front of the packet.
    srpPkt->appendMediaFragment(headerFrag);

    // Add data fragments
    for (uint32 i = 0; i < pPkt->getNumFragments(); i++)
    {
        pPkt->getMediaFragment(i, headerFrag);
        srpPkt->appendMediaFragment(headerFrag);
    }

    // Initialize crc calcuration
    CrcClear();
    // Calculate crc
    crc = iCrc.Crc16Check(srpPkt);   // 16bit CRC Infomation Create

    // Set crc to fcs
    CrcSetToFrame(crc, (uint8 *) FCSFrag.getMemFragPtr());
    FCSFrag.getMemFrag().len = SRP_FCS_SIZE;

    // Add CRC to the end of the packet.
    srpPkt->appendMediaFragment(FCSFrag);

    txData = PVMFMediaData::createMediaData(srpPkt, iOutstandingTxMediaMsgPoolAlloc);
    if (!txData)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0x40000020, "SRP::SrpCommandCreate - Unable allocate tx message %d", error))
        return txData;
    }
    //If SRP
    if (header == SRP_COMMAND_HEADER)
    {
        txData->setFormatSpecificInfo(iSrpNsrpEntryNumFrag);
    }
    //Else WNSRP
    else
    {
        txData->setFormatSpecificInfo(iWnsrpEntryNumFrag);
    }

    // Set sequence number
    txData->setSeqNum(pPkt->getSeqNum());

    iStats.totalCCSRLChunksSent++;
    iStats.totalCommandFramesSent++;

    /* Record initial time when chunk is sent. */
    iInitialTimeChunkSent.set_to_current_time();

    return txData;
}

/************************************************************************/
/*  function name       : SrpCommandClear                               */
/*  function outline    : Clear comand frame for re-transmittion        */
/*  function discription: void SrpCommandClear(SRPRespTimer &timer)		*/
/*  input data          : timer - response timer that hold frame to be	*/
/*								  retransmitted.						*/
/*  output data         : None                                          */
/*  draw time           : '05.08.24	                                    */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                          Copyright (C) 2005 PacketVideo Corp.        */
/************************************************************************/
void SRP::SrpCommandClear(uint32 seqNum)
{
    uint32 i;

    //Find packet with same sequence number in wait queue and remove
    for (i = 0; i < iSrpWaitQueue.size(); i++)
    {
        if (iSrpWaitQueue[i]->getSeqNum() == seqNum)
        {
            iSrpWaitQueue.erase(&iSrpWaitQueue[i]);
            return;
        }
    }

    return;
}

/************************************************************************/
/*  function name       : SrpResponseCreate                             */
/*  function outline    : Create response frame                         */
/*  function discription: void SrpResponseCreate( uint8 seqNum )        */
/*  input data          : seqNum - Sequence number of response frame    */
/*						  header - SRP response frame type				*/
/*  output data         : pointer to created response packet            */
/*  draw time           : '96.10.29                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
PVMFSharedMediaDataPtr SRP::SrpResponseCreate(uint8 seqNum, uint8 header)
{
    OsclSharedPtr<PVMFMediaDataImpl> resp;
    PVMFSharedMediaDataPtr mediaData;
    int32 error = OsclErrNone;
    uint8* pEditPtr;
    uint16 crc;
    uint16 Crc16Check(uint8 *, uint16);
    OsclRefCounterMemFrag frag;

    OSCL_UNUSED_ARG(error);

    switch (header)
    {
        case NSRP_RESPONSE_HEADER:
        case WNSRP_RESPONSE_HEADER:
            resp = iNsrpRespPacketAlloc->allocate(SRP_HEADER_SIZE + SRP_FCS_SIZE + SRP_SEQUENCE_SIZE); /* WWU_NSRP */
            break;

        case SRP_RESPONSE_HEADER:
            resp = iRespPacketAlloc->allocate(SRP_HEADER_SIZE + SRP_FCS_SIZE); /* SRP */
            break;

        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0x40000020, "SRP::SrpResponseCreate - Unknown response type %d", header));
            return mediaData;
    }

    //Unable to allocate fragment.
    if (!resp)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0x40000020, "SRP::SrpResponseCreate - Unable to allocate fragment %d", error));
        return mediaData;
    }

    resp->getMediaFragment(0, frag);

    pEditPtr = (uint8 *) frag.getMemFragPtr();

    /* Initialize crc calcuration */
    CrcClear() ;

    /* Set header octet */
    switch (header)
    {
        case NSRP_RESPONSE_HEADER:
        case WNSRP_RESPONSE_HEADER:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_VERBOSE, (0x40000020, "SRP::SrpResponseCreate - Sending NSRP or WNSRP Response, SeqNum = %d", seqNum));
            *pEditPtr++ = header;
            *pEditPtr++ = seqNum;
            resp->setMediaFragFilledLen(0, SRP_HEADER_SIZE + SRP_FCS_SIZE + SRP_SEQUENCE_SIZE);
            crc = iCrc.Crc16Check((uint8 *) frag.getMemFragPtr(), SRP_HEADER_SIZE + SRP_SEQUENCE_SIZE);	     /* 16bit CRC Infomation Create	    */
            break;

        case SRP_RESPONSE_HEADER:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_VERBOSE, (0x40000020, "SRP::SrpResponseCreate - Sending SRP Response"));
            *pEditPtr++ = header;
            resp->setMediaFragFilledLen(0, SRP_HEADER_SIZE + SRP_FCS_SIZE);
            crc = iCrc.Crc16Check((uint8 *) frag.getMemFragPtr(), SRP_HEADER_SIZE);	      /* 16bit CRC Infomation Create	    */
            break;

        default:
            return mediaData;
    }

    /* Set crc to fcs */
    CrcSetToFrame(crc, pEditPtr) ;

    mediaData = PVMFMediaData::createMediaData(resp, iRespMediaMsgPoolAlloc);
    if (!mediaData)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0x40000020, "SRP::SrpResponseCreate - Unable to allocate msg %d", error));
        return mediaData;
    }
    mediaData->setSeqNum((uint32) seqNum);
    //If SRP
    switch (header)
    {
        case NSRP_RESPONSE_HEADER:
        case SRP_RESPONSE_HEADER:
            mediaData->setFormatSpecificInfo(iSrpNsrpEntryNumFrag);
            break;

        case WNSRP_RESPONSE_HEADER:
            mediaData->setFormatSpecificInfo(iWnsrpEntryNumFrag);
            break;
    }

    return mediaData;
}

/************************************************************************/
/*  function name       : SrpMsgCopy                                    */
/*  function outline    : Get SRP data from in packet and put it in the */
/*                        out packet.                                   */
/*  function discription: bool SrpMsgCopy(								*/
/*										PVMFSharedMediaDataPtr inPkt)	*/
/*  input data          : inPkt - Pointer to incoming packet.			*/
/*                        outPkt - Pointer to outgoing packet.          */
/*  output data         : None                                          */
/*  draw time           : '96.10.29                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
bool SRP::SrpMsgCopy(PVMFSharedMediaDataPtr inPkt)
{
    OsclRefCounterMemFrag frag;
    OsclSharedPtr<PVMFMediaDataImpl> data;
    SRPRxData *rxData;
    int32 error = OsclErrNone;
    OSCL_UNUSED_ARG(error);
    uint8 *pData;
    uint8* pFragPos = NULL;
    int fragLen = 0;

    inPkt->getMediaFragment(0, frag);

    pData = ((uint8 *) frag.getMemFragPtr()) + SRP_HEADER_SIZE + SRP_SEQUENCE_SIZE;

    //If using CCSRL method.
    if (iCcsrlSduSize > 0)
    {
        iStats.totalCCSRLChunksRecv++;
        pFragPos = pData + CCSRL_HEADER_SIZE;
        fragLen = frag.getMemFragSize() - SRP_HEADER_SIZE - SRP_SEQUENCE_SIZE - CCSRL_HEADER_SIZE - SRP_FCS_SIZE;
    }
    else
    {
        pFragPos = pData;
        fragLen = frag.getMemFragSize() - SRP_HEADER_SIZE - SRP_SEQUENCE_SIZE - SRP_FCS_SIZE;
    }

    //Data to large.  Pass up whatever is buffered in receive packet
    if (fragLen > MAX_SIZE_OF_SRP_PDU)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0x40000020, "SRP::SrpMsgCopy - incoming data to large to copy %d, max %d", fragLen, MAX_SIZE_OF_SRP_PDU));
        return false;
    }

    rxData = GetRxFrag();

    if (rxData == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0x40000020, "SRP::SrpMsgCopy - unable to allocate rx data frag %d", error));
        return false;
    }

    if (!AllocateRxPacket(data, rxData))
        return 0;
    if (!CreateMediaData(rxData, data))
        return 0;
    rxData->data->getMediaFragment(0, frag);
    oscl_memcpy(frag.getMemFragPtr(), pFragPos, fragLen);
    rxData->data->setMediaFragFilledLen(0, fragLen);

    rxData->seqNum = (uint8) inPkt->getSeqNum();

    if (iCcsrlSduSize > 0)
    {
        // Return true if last CCSRL fragment in original H245 packet.
        data->setMarkerInfo(((*pData == LAST_CCSRL_CHUNK) ? true : false));
    }
    else
    {
        data->setMarkerInfo(true);
    }

    //Add to rx pending list
    AddPendingRxFrag(rxData);

    return true;
}

bool SRP::AllocateRxPacket(OsclSharedPtr<PVMFMediaDataImpl>& data, SRPRxData* rxData)
{
    data = iRxPacketAlloc->allocate(MAX_SIZE_OF_SRP_PDU);
    if (!data)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0x40000020, "SRP::SrpMsgCopy - Unable allocate rx message impl"));
        FreeRxFrag(rxData);
        return false;
    }
    return true;

}
bool SRP::CreateMediaData(SRPRxData* rxData, OsclSharedPtr<PVMFMediaDataImpl> data)
{
    rxData->data = PVMFMediaData::createMediaData(data, iRxMediaMsgPoolAlloc);
    if (!(rxData->data))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0x40000020, "SRP::SrpMsgCopy - Unable allocate rx message"));
        FreeRxFrag(rxData);
        return false;
    }
    return true;

}


bool SRP::CreateMediaData(PVMFSharedMediaDataPtr& srpPkt,
                          OsclSharedPtr<PVMFMediaDataImpl> data)
{
    srpPkt = PVMFMediaData::createMediaData(data, iTxMediaMsgPoolAlloc);
    if (!srpPkt)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0x40000020, "SRP::UpperLayerRx - Unable allocate tx message"));
        return false;
    }
    return true;
}

/************************************************************************/
/*  function name       : Srp_0_1                                       */
/*  function outline    : PDU received in IDLE                          */
/*  function discription: PVMFSharedMediaDataPtr Srp_0_1()				*/
/*  input data          : pPkt - pointer of received packet from corder */
/*  output data         : None                                          */
/*  draw time           : '96.10.29                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
int SRP::Srp_0_1()
{
    PVMFSharedMediaDataPtr pSrpCommandPkt;
    PVMFSharedMediaDataPtr pTxPkt;
    PVMFSharedMediaDataPtr pFirstPkt;
    SRPRespTimer *pTimer;

    switch (WnsrpStatusGet())
    {
        case NO_WNSRP_SUPPORT:
        case WNSRP_TX_SUPPORT:
            pFirstPkt = SrpSendWaitGet(0);

            /* Create SRP command */
            pTxPkt = SrpCommandCreate(pFirstPkt, SRP_COMMAND_HEADER);

            /* If packet could not be created. */
            if (pTxPkt.GetRep() == NULL)
            {
                return(0);
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0x40000020, "SRP::Srp_0_1 - lowerlayertx srp cmd seq num %d", pTxPkt->getSeqNum()));

            /* Send SRP command */
            LowerLayerTx(pTxPkt);
            /* Start response wait timer */
            SrpT401Start(iSrpCommandSave, pTxPkt);

            if (WnsrpStatusGet() == NO_WNSRP_SUPPORT) break;
            //If tx support only, must send srp frame also.

        case WNSRP_FULL_SUPPORT:
            while (!iFreeWnsrpCommandSave.empty() &&
                    (iActiveWnsrpCommandSave.size() < iSrpWaitQueue.size()))
            {
                pSrpCommandPkt = SrpSendWaitGet(iActiveWnsrpCommandSave.size());

                //No more outstanding frags to send
                if (pSrpCommandPkt.GetRep() == NULL)
                {
                    break;
                }

                /* Create WNSRP command */
                pTxPkt = SrpCommandCreate(pSrpCommandPkt, WNSRP_COMMAND_HEADER);

                /* If packet could not be created. */
                if (pTxPkt.GetRep() == NULL)
                {
                    return(0);
                }

                pTimer = GetFreeWnsrpTimer();

                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0x40000020, "SRP::Srp_0_1 - lowerlayertx wnsrp cmd seq num %d", pTxPkt->getSeqNum()));

                /* Send SRP command */
                LowerLayerTx(pTxPkt);
                /* Start response wait timer */
                SrpT401Start(*pTimer, pTxPkt);

                AddActiveWnsrpTimer(pTimer);
            }

            break;

        default:
            break;
    }

    /* Set status */
    SrpStatusSet(STS_WAIT_RESPONSE);
    return (1);
}

/************************************************************************/
/*  function name       : Srp_0_2                                       */
/*  function outline    : SRP command received                          */
/*                                    in IDLE or WAITING-RESPONSE       */
/*  function discription: PVMFSharedMediaDataPtr Srp_0_2(				*/
/*										PVMFSharedMediaDataPtr pPkt )   */
/*  input data          : pPkt - pointer of received packet from AL1C   */
/*  output data         : None                                          */
/*  draw time           : '96.10.29                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
int SRP::Srp_0_2(PVMFSharedMediaDataPtr pPkt, int event)
{
    PVMFSharedMediaDataPtr resp;
    OsclRefCounterMemFrag frag;
    SRPRxData *rxData;
    uint8 header;

    //If WNSRP command
    if (event == EV_WNSRP_COMMAND)
    {
        header = WNSRP_RESPONSE_HEADER;
    }
    //Else SRP command
    else
    {
        if (iUseNSRP)
        {
            header = NSRP_RESPONSE_HEADER;
        }
        else
        {
            header = SRP_RESPONSE_HEADER;
        }
    }

    if (iFirstCmd)
    {
        //WNSRP must always start with 0
        if (event == EV_WNSRP_COMMAND)
        {
            SrpRecvSeqSet(0);
        }
        else
        {
            SrpRecvSeqSet((uint8) pPkt->getSeqNum());
        }

        iFirstCmd = false;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_VERBOSE, (0x40000020, "SRP::Srp_0_2 - Incoming SequenceNumber = %d", pPkt->getSeqNum()));

    if (CheckRxSeqNum(pPkt->getSeqNum(), header))
    {
        if (!SrpMsgCopy(pPkt))
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0x40000020, "SRP::Srp_0_2 - Unable to copy data"));
            return 0;
        }

        // Create SRP response
        resp = SrpResponseCreate((uint8) pPkt->getSeqNum(), header);
        if (resp.GetRep() == NULL)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0x40000020, "SRP::Srp_0_2 - Unable create reponse packet"));
            /* If unable to create response message. */
            return 0;
        }

        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0x40000020, "SRP::Srp_0_2 - lowerlayertx srp resp seq num %d", resp->getSeqNum()));

        // SRP response send
        LowerLayerTx(resp);

        // Check for full H245 messages
        while (iPendingRxFragList &&
                (iPendingRxFragList->seqNum == SrpRecvSeqGet()))
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0x40000020, "SRP::Srp_0_2 - upperlayertx last frag %d", iPendingRxFragList->data->getMarkerInfo()));

            UpperLayerTx(iPendingRxFragList->data);

            rxData = iPendingRxFragList;
            iPendingRxFragList = iPendingRxFragList->next;

            FreeRxFrag(rxData);

            SrpRecvSeqCount();
        }
    }

    return 1;
}

/************************************************************************/
/*  function name       : Srp_1_1                                       */
/*  function outline    : PDU received from corder in WAITING-RESPONSE  */
/*  function discription: PVMFSharedMediaDataPtr Srp_1_1()				*/
/*  input data          : pPkt - pointer of received packet from corder */
/*  output data         : None                                          */
/*  draw time           : '96.10.29                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
int SRP::Srp_1_1()
{
    PVMFSharedMediaDataPtr pTxPkt;
    SRPRespTimer *pTimer;

    switch (WnsrpStatusGet())
    {
        case NO_WNSRP_SUPPORT:
            //SRP mode, cannot send until response for last packet is received.
            break;

        case WNSRP_TX_SUPPORT:
        case WNSRP_FULL_SUPPORT:
            while (!iFreeWnsrpCommandSave.empty() &&
                    (iActiveWnsrpCommandSave.size() < iSrpWaitQueue.size()))
            {
                /* Create WNSRP command */
                pTxPkt = SrpCommandCreate(SrpSendWaitGet(iActiveWnsrpCommandSave.size()), WNSRP_COMMAND_HEADER);

                /* If packet could not be created. */
                if (pTxPkt.GetRep() == NULL)
                {
                    return(0);
                }

                pTimer = GetFreeWnsrpTimer();

                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0x40000020, "SRP::Srp_1_1 - lowerlayertx wnsrp cmd seq num %d", pTxPkt->getSeqNum()));

                /* Send SRP command */
                LowerLayerTx(pTxPkt);
                /* Start response wait timer */
                SrpT401Start(*pTimer, pTxPkt);

                AddActiveWnsrpTimer(pTimer);
            }
            break;

        default:
            break;
    }

    return (1);
}

/************************************************************************/
/*  function name       : Srp_1_3                                       */
/*  function outline    : SRP response received in WAITING-RESPONSE     */
/*  function discription: PVMFSharedMediaDataPtr Srp_1_3(				*/
/*										PVMFSharedMediaDataPtr pPkt )   */
/*  input data          : pPkt - pointer of received packet from AL1C   */
/*  output data         : None                                          */
/*  draw time           : '96.10.29                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
int SRP::Srp_1_3(PVMFSharedMediaDataPtr pPkt, int event)
{
    PVMFSharedMediaDataPtr pSrpCommandPkt;
    SRPRespTimer *pTimer;
    uint32 i;
    uint32 seqNum = pPkt->getSeqNum();
    bool isWaiting;

    switch (event)
    {
        case EV_WNSRP_RESPONSE:
            // Remove packet from waiting list
            SrpCommandClear(seqNum);

            for (isWaiting = false, i = 0; i < iActiveWnsrpCommandSave.size(); i++)
            {
                if (seqNum == iActiveWnsrpCommandSave[i]->pPkt->getSeqNum())
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0x40000020, "SRP::Srp_1_3 - wnsrp timer found %d", i));

                    // Resend previous WNSRP packets per section A.4.3
                    for (uint32 k = 0; k < i; k++)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0x40000020, "SRP::Srp_1_3 - lowerlayertx EV_WNSRP_RESPONSE wnsrp cmd seq num %d, index k %d", iActiveWnsrpCommandSave[k]->pPkt->getSeqNum(), k));

                        // Send SRP command
                        LowerLayerTx(iActiveWnsrpCommandSave[k]->pPkt);
                        // Stop timer
                        SrpT401Stop(*iActiveWnsrpCommandSave[k]);
                        // Restart timer
                        SrpT401Start(*iActiveWnsrpCommandSave[k], iActiveWnsrpCommandSave[k]->pPkt);
                        // Count-up retry counter
                        ++iActiveWnsrpCommandSave[k]->N400Counter;
                    }

                    // Stop response wait timer
                    SrpT401Stop(*iActiveWnsrpCommandSave[i]);

                    // Release packet
                    iActiveWnsrpCommandSave[i]->pPkt.Unbind();

                    // Remove timer from active list
                    RemoveActiveWnsrpTimer(iActiveWnsrpCommandSave[i]);

                    isWaiting = true;
                    break;
                }
            }

            if (!isWaiting)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING, (0x40000020, "SRP::Srp_1_3 - WNSRP command with seq %d not waiting for response", seqNum));
                return 0;
            }
            break;

        case EV_NSRP_RESPONSE:
            // Remove packet from waiting list
            SrpCommandClear(seqNum);

            //Stop any wnsrp timers with same seq number
            for (i = 0; i < iActiveWnsrpCommandSave.size(); i++)
            {
                if (seqNum == iActiveWnsrpCommandSave[i]->pPkt->getSeqNum())
                {
                    // Stop response wait timer
                    SrpT401Stop(*iActiveWnsrpCommandSave[i]);

                    // Release packet
                    iActiveWnsrpCommandSave[i]->pPkt.Unbind();

                    // Remove timer from active list
                    RemoveActiveWnsrpTimer(iActiveWnsrpCommandSave[i]);
                    break;
                }
            }

            if (iSrpCommandSave.isTimerActive)
            {
                if (seqNum == iSrpCommandSave.pPkt->getSeqNum())
                {
                    // Stop response wait timer
                    SrpT401Stop(iSrpCommandSave);

                    // Release packet
                    iSrpCommandSave.pPkt.Unbind();
                }
                else
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING, (0x40000020, "SRP::Srp_1_3 - NSRP command seq %d does not match  NSRP resp seq %d", iSrpCommandSave.pPkt->getSeqNum(), seqNum));
                    return 0;
                }
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING, (0x40000020, "SRP::Srp_1_3 - No command waiting for NSRP response"));
                return 0;
            }
            break;

        case EV_RESPONSE:
            if (iSrpCommandSave.isTimerActive)
            {
                //Stop oldest wnsrp timer
                if (!iActiveWnsrpCommandSave.empty())
                {
                    // Stop response wait timer
                    SrpT401Stop(*iActiveWnsrpCommandSave[0]);

                    // Release packet
                    iActiveWnsrpCommandSave[0]->pPkt.Unbind();

                    // Remove timer from active list
                    RemoveActiveWnsrpTimer(iActiveWnsrpCommandSave[0]);
                }

                // Remove packet from waiting list
                SrpCommandClear(iSrpCommandSave.pPkt->getSeqNum());

                // Stop response wait timer
                SrpT401Stop(iSrpCommandSave);

                // Release packet
                iSrpCommandSave.pPkt.Unbind();
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING, (0x40000020, "SRP::Srp_1_3 - No command waiting for SRP response"));
                return 0;
            }
            break;

        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0x40000020, "SRP::Srp_1_3 - Invalid response type %d", event));
            return 0;
    }

    switch (WnsrpStatusGet())
    {
        case NO_WNSRP_SUPPORT:
        case WNSRP_TX_SUPPORT:
            // Waiting PDU exists?
            if (!iSrpWaitQueue.empty())
            {
                // Create SRP command
                pSrpCommandPkt = SrpCommandCreate(SrpSendWaitGet(0), SRP_COMMAND_HEADER);

                if (pSrpCommandPkt.GetRep() == NULL)
                {
                    // Set status
                    SrpStatusSet(STS_IDLE);
                    return 0;
                }

                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0x40000020, "SRP::Srp_1_3 - lowerlayertx srp cmd seq num %d", pSrpCommandPkt->getSeqNum()));

                // Send SRP command
                LowerLayerTx(pSrpCommandPkt);
                // Start response wait timer
                SrpT401Start(iSrpCommandSave, pSrpCommandPkt);
            }

            if (WnsrpStatusGet() == NO_WNSRP_SUPPORT)
            {
                break;
            }

            //Fall through to next case

        case WNSRP_FULL_SUPPORT:
            //Check if wnsrp timer is available
            if (iActiveWnsrpCommandSave.size() < iSrpWaitQueue.size())
            {
                /* Create WNSRP command */
                pSrpCommandPkt = SrpCommandCreate(SrpSendWaitGet(iActiveWnsrpCommandSave.size()), WNSRP_COMMAND_HEADER);

                /* If packet could not be created. */
                if (pSrpCommandPkt.GetRep() == NULL)
                {
                    return(0);
                }

                pTimer = GetFreeWnsrpTimer();

                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0x40000020, "SRP::Srp_1_3 - lowerlayertx WNSRP_FULL_SUPPORT wnsrp cmd seq num %d", pSrpCommandPkt->getSeqNum()));

                /* Send SRP command */
                LowerLayerTx(pSrpCommandPkt);
                /* Start response wait timer */
                SrpT401Start(*pTimer, pSrpCommandPkt);

                AddActiveWnsrpTimer(pTimer);
            }


            break;
    }

    if (!iSrpCommandSave.isTimerActive && iActiveWnsrpCommandSave.empty())
    {
        // Set status
        SrpStatusSet(STS_IDLE);
    }

    /* Get total response time for frame. */
    UpdateRespStats();

    return(1) ;
}

/************************************************************************/
/*  function name       : Srp_1_4                                       */
/*  function outline    : Response wait timer timeout                   */
/*                                            in WAITING-RESPONSE       */
/*  function discription: PVMFSharedMediaDataPtr Srp_1_4(				*/
/*										PVMFSharedMediaDataPtr pPkt,	*/
/*										int32 id  )						*/
/*  input data          : pPkt - pointer of timeout information         */
/*						  id - id of timer that timed out				*/
/*  output data         : None                                          */
/*  draw time           : '96.10.29                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
int SRP::Srp_1_4(int32 id)
{
    PVMFSharedMediaDataPtr pSrpCommandPkt;
    SRPRespTimer *timer;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING, (0x40000020, "SRP::Srp_1_4 - Response wait timer timeout"));

    if (iSrpCommandSave.timerID == id)
    {
        timer = &iSrpCommandSave;
    }
    else if ((uint32) id < iWNSRPTxWindow)
    {
        timer = FindActiveWnsrpTimer(id);
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0x40000020, "SRP::Srp_1_4 - unknown SRP timer %d", id));
        return 0;
    }

    // Retry over ?
    if (SrpN400Check(*timer) == false)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING, (0x40000020, "SRP::Srp_1_4 - Max retry over"));

        // Remove command from wait queue.
        SrpCommandClear(timer->pPkt->getSeqNum());

        // Stop timer
        SrpT401Stop(*timer);

        //Release packet
        timer->pPkt.Unbind();

        // Take timer off WNSRP timer wait list (if it is a WNSRP timer)
        RemoveActiveWnsrpTimer(timer);

        // If observer exists.


        if (iObserver)
        {
            /* Report transmission failure to TSC.
               TSC will stop SRP if necessary. */
            iObserver->TransmissionFailure();
        }

        else
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0x40000020, "SRP::Srp_1_4 - No observer!"));

            switch (WnsrpStatusGet())
            {
                case NO_WNSRP_SUPPORT:
                    // Set status
                    SrpStatusSet(STS_IDLE);
                    break;

                case WNSRP_TX_SUPPORT:
                case WNSRP_FULL_SUPPORT:
                    //If no more packets waiting to be sent.
                    if (!iSrpWaitQueue.empty())
                    {
                        // Set status
                        SrpStatusSet(STS_IDLE);
                    }
                    break;
            }
        }
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING, (0x40000020, "SRP::Srp_1_4 - Create and send SRP command again, seq num %d", timer->pPkt->getSeqNum()));
        // Send SRP command
        LowerLayerTx(timer->pPkt);
        // Count-up retry counter
        ++timer->N400Counter;
    }

    return 1;
}

/************************************************************************/
/*  function name       : SrpSendWaitGet                                */
/*  function outline    : Get from queue for waiting transmittion       */
/*  function discription: PVMFSharedMediaDataPtr SrpSendWaitGet(		*/
/*														uint32 index)   */
/*  input data          : index - index of srp frags                    */
/*  output data         : data pointer of waiting transmittion          */
/*  draw time           : '05.08.25                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                          Copyright (C) 2005 PacketVideo Corp.        */
/************************************************************************/
PVMFSharedMediaDataPtr SRP::SrpSendWaitGet(uint32 index)
{
    if (index < iSrpWaitQueue.size())
    {
        return iSrpWaitQueue[index];
    }
    else
    {
        PVMFSharedMediaDataPtr temp;
        return temp;
    }
}

/************************************************************************/
/*  function name       : SrpSendWaitRemove                             */
/*  function outline    : Remove from queue for waiting transmission    */
/*  function discription: PVMFSharedMediaDataPtr SrpSendWaitRemove(		*/
/*														uint32 index)	*/
/*  input data          : index - index of srp frags                    */
/*  output data         : data pointer of waiting transmittion          */
/*  draw time           : '05.08.25                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                          Copyright (C) 2005 PacketVideo Corp.        */
/************************************************************************/
int SRP::SrpSendWaitRemove(uint32 index)
{
    if (index < iSrpWaitQueue.size())
    {
        iSrpWaitQueue.erase(&iSrpWaitQueue[index]);
        return 1;
    }
    else
    {
        return 0;
    }
}

/************************************************************************/
/*  function name       : SrpSendSeqCount                               */
/*  function outline    : Count up sequence number for sending          */
/*  function discription: void SrpSendSeqCount( void )                  */
/*  input data          : None                                          */
/*  output data         : None                                          */
/*  draw time           : '96.10.29                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void SRP::SrpSendSeqCount()
{
    if (iSendSeqNumber < 255)
    {
        ++ iSendSeqNumber;
    }
    else
    {
        iSendSeqNumber = 0;
    }
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_VERBOSE, (0x40000020, "SRP::SrpSendSeqCount = %d", iSendSeqNumber));
    return;
}


/************************************************************************/
/*  function name       : SrpRecvSeqCount                               */
/*  function outline    : Count up sequence number for receiving        */
/*  function discription: void SrpRecvSeqCount( void )                  */
/*  input data          : None                                          */
/*  output data         : None                                          */
/*  draw time           : '96.10.29                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void SRP::SrpRecvSeqCount()
{
    if (iRecvSeqNumber < 255)
    {
        ++iRecvSeqNumber;
    }
    else
    {
        iRecvSeqNumber = 0;
    }

    if (iOldestWNSRPRetransSeqNum < 255)
    {
        ++iOldestWNSRPRetransSeqNum;
    }
    else
    {
        iOldestWNSRPRetransSeqNum = 0;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_VERBOSE, (0x40000020, "SRP::SrpRecvSeqCount = %d, oldest retrans = %d", iRecvSeqNumber, iOldestWNSRPRetransSeqNum));
    return;
}


/************************************************************************/
/*  function name       : SrpStatusSet                                  */
/*  function outline    : Status number set                             */
/*  function discription: void SrpStatusSet( SrpStatus newStatus )      */
/*  input data          : newStatus - status number                     */
/*  output data         : None                                          */
/*  draw time           : '96.10.29                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void SRP::SrpStatusSet(SrpStatus newStatus)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_VERBOSE, (0x40000020, "SRP::SrpStatusSet - SrpStatus from %d => %d", iStatus, newStatus));
    iStatus = newStatus;
    return;
}

/************************************************************************/
/*  function name       : WnsrpStatusSet                                */
/*  function outline    : WNSRP Status number set                       */
/*  function discription: void WnsrpStatusSet( WnsrpStatus newStatus )  */
/*  input data          : newStatus - status number                     */
/*  output data         : None                                          */
/*  draw time           : '05.08.23                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                          Copyright (C) 2005 PacketVideo Corp.        */
/************************************************************************/
void SRP::WnsrpStatusSet(WnsrpStatus newStatus)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_VERBOSE, (0x40000020, "SRP::WnsrpStatusSet - WnsrpStatus from %d => %d", iCurWnsrpStatus, newStatus));
    iCurWnsrpStatus = newStatus;

    switch (iCurWnsrpStatus)
    {
        case NO_WNSRP_SUPPORT:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO, (0x40000020, "SRP::WnsrpStatusSet - Remote does not support WNSRP!"));
            //Clear all WNSRP resp timers
            while (!iActiveWnsrpCommandSave.empty())
            {
                // Stop response wait timer
                SrpT401Stop(*iActiveWnsrpCommandSave[0]);

                // Release packet
                iActiveWnsrpCommandSave[0]->pPkt.Unbind();

                // Remove timer from active list
                RemoveActiveWnsrpTimer(iActiveWnsrpCommandSave[0]);
            }

            if (iObserver) iObserver->UseWNSRP(false);
            break;

        case WNSRP_FULL_SUPPORT:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO, (0x40000020, "SRP::WnsrpStatusSet - Remote supports WNSRP!"));
            //Disable SRP tx
            if (iSrpCommandSave.isTimerActive)
            {
                // Stop response wait timer
                SrpT401Stop(iSrpCommandSave);

                // Release packet
                iSrpCommandSave.pPkt.Unbind();
            }

            //Send all WNSRP frames using the default multiplex table entry number 0
            *(uint8 *)iWnsrpEntryNumFrag.getMemFragPtr() = SRP_NSRP_MT_ENTRY_NUMBER;

            if (iObserver) iObserver->UseWNSRP(true);
            break;
        default:
            break;
    }
    return;
}

/************************************************************************/
/*  function name       : SrpT401Start                                  */
/*  function outline    : T401 timer start                              */
/*  function discription: void SrpT401Start( SRPRespTimer &timer )      */
/*  input data          : timer - timer to start                        */
/*  output data         : None                                          */
/*  draw time           : '05.08.24                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                          Copyright (C) 2005 PacketVideo Corp.        */
/************************************************************************/
void SRP::SrpT401Start(SRPRespTimer &timer, PVMFSharedMediaDataPtr pPkt)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "SRP::SrpT401Start - id %d, IsPrimaryTimerActive = %d", timer.timerID, timer.isTimerActive));

    /* Start timer that is not active. */
    if (!timer.isTimerActive)
    {
        timer.T401Timer.Request(timer.timerID, (int32)this, iT401TimerValue, this, true);
        timer.isTimerActive = true;
    }

    timer.pPkt = pPkt;
    timer.N400Counter = 0;

    return;
}

/************************************************************************/
/*  function name       : SrpT401Stop                                   */
/*  function outline    : T401 timer stop                               */
/*  function discription: void SrpT401Stop( SRPRespTimer &timer )       */
/*  input data          : timer - timer to stop                         */
/*  output data         : None                                          */
/*  draw time           : '05.08.24                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                          Copyright (C) 2005 PacketVideo Corp.        */
/************************************************************************/
void SRP::SrpT401Stop(SRPRespTimer &timer)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0x40000020, "SRP::SrpT401Stop - id %d, IsPrimaryTimerActive = %d", timer.timerID, timer.isTimerActive));

    /* Reset active timer */
    if (timer.isTimerActive)
    {
        timer.T401Timer.Cancel(timer.timerID);
        timer.isTimerActive = false;
    }

    return;
}

/************************************************************************/
/*  function name       : SrpT401Timeout                                */
/*  function outline    : T401 timeout                                  */
/*  function discription: int SrpT401Timeout(int TimerId, void *Param)  */
/*  input data          : TimerId - timer id                            */
/*                        Param - timer parameter                       */
/*  output data         : 0 - Reset timer  to fire again                */
/*						  1 - Do not reset timer						*/
/*  draw time           : '96.10.29                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
// OsclTimerObserver virtual function
void SRP::TimeoutOccurred(int32 timerID, int32 param)
{
    OSCL_UNUSED_ARG(param);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0x40000020, "SRP::TimeoutOccurred, id %d, wnsrp status %d", timerID, iCurWnsrpStatus));

    iStats.totalNumTimeouts++;

    /* Process call */
    SrpStateChange(SrpStatusGet(), EV_TIMEOUT, (void *) &timerID);
}


/************************************************************************/
/*  function name       : LowerLayerRx									*/
/*  function outline    : Lower Layer Receive                           */
/*  function description: void LowerLayerRx(MediaPacket* pPkt)			*/
/*  input data          : pPkt - pointer to incoming packet             */
/*  output data         : None                                          */
/*  draw time           : '96.10.29                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void SRP::LowerLayerRx(PVMFSharedMediaDataPtr pPkt)
{
    int32 error;
    OsclSharedPtr<PVMFMediaDataImpl> data;
    OSCL_TRY(error, data = iRxPacketAlloc->allocate(MAX_SIZE_OF_SRP_PDU));
    OSCL_FIRST_CATCH_ANY(error, PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0x40000020, "SRP::LowerLayerRx - Unable allocate rx message impl %d", error));
                         return);

    OsclRefCounterMemFrag frag;
    data->getMediaFragment(0, frag);
    uint8* pos = (uint8*)frag.getMemFragPtr();
    for (unsigned i = 0;i < pPkt->getNumFragments();i++)
    {
        OsclRefCounterMemFrag src_frag;
        pPkt->getMediaFragment(i, src_frag);
        oscl_memcpy(pos, src_frag.getMemFragPtr(), src_frag.getMemFragSize());
        pos += src_frag.getMemFragSize();
    }
    data->setMediaFragFilledLen(0, pPkt->getFilledSize());
    /* Record number of frames received from lower layer. */
    iStats.totalFramesRecv++;

    /* Record number of bytes received. */
    iStats.totalBytesRecv += pPkt->getFilledSize();

    /* Process call */
    PVMFSharedMediaDataPtr mediaData;
    OSCL_TRY(error, mediaData = PVMFMediaData::createMediaData(data, iRxMediaMsgPoolAlloc));
    OSCL_FIRST_CATCH_ANY(error, PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0x40000020, "SRP::LowerLayerRx - Unable allocate rx message %d", error));
                         return);
    SrpStateChange(SrpStatusGet(), SrpGetEventNoFromAL1(mediaData), &mediaData);
    return;

}

/************************************************************************/
/*  function name       : UpperLayerRx									*/
/*  function outline    : Upper Layer Receive                           */
/*  function description: void UpperLayerRx(MediaPacket* pPkt)			*/
/*  input data          : pPkt - pointer to incoming packet             */
/*  output data         : None                                          */
/*  draw time           : '96.10.29                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void SRP::UpperLayerRx(PVMFSharedMediaDataPtr pPkt)
{
    uint32 offset;
    uint8 *buf;
    PVMFSharedMediaDataPtr srpPkt;
    OsclRefCounterMemFrag newFrag;
    OsclRefCounterMemFrag CCSRLFrag;
    OsclRefCounterMemFrag frag;
    OsclSharedPtr<PVMFMediaDataImpl> data;
    uint8 seqNum;
    uint32 size;

    iStats.totalH245MessagesToSend++;


    // IMPORTANT: Assume upper layer is sending 1 fragment in a packet!
    if (pPkt->getNumFragments() != 1)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0x40000020, "SRP::UpperLayerRx - Num frags of incoming message not equal to 1, %d", pPkt->getNumFragments()));
        return;
    }

    pPkt->getMediaFragment(0, frag);

    /* CCSRL */
    if (iCcsrlSduSize > 0)
    {
        //---------------------------------------------------------
        // Send the encoded message (pCodeData, GetSize) to SRP
        //   using the multi-chunk feature of H.324.
        //---------------------------------------------------------
        size = frag.getMemFragSize();

        // If queue cannot hold the number of fragments that will be needed.
        // If not enough messages available to send the entire message
        if (((size / iCcsrlSduSize) + 1) > iTxPacketAlloc->NumMsgAvail())
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0x40000020, "SRP::UpperLayerRx - Unable allocate enough messages, current num %d", iTxPacketAlloc->NumMsgAvail()));
            return;
        }

        if (size > iCcsrlSduSize)
        {
            iStats.totalH245MessagesFragmented++;
        }

        buf = (uint8 *) frag.getMemFragPtr();
        offset = 0;

        while (offset < size)
        {
            if (!Allocate(data, CCSRLFrag))
                return;
            newFrag = frag;
            newFrag.getMemFrag().ptr = (buf + offset);

            /* Set sequence number */
            seqNum = (uint8)SrpSendSeqGet();
            SrpSendSeqCount();
            *(uint8 *)CCSRLFrag.getMemFrag().ptr = seqNum;
            CCSRLFrag.getMemFrag().len = SRP_SEQUENCE_SIZE + CCSRL_HEADER_SIZE;

            if ((offset + iCcsrlSduSize) >= size)
            {
                /* Last CCSRL chunk */
                *((uint8 *)CCSRLFrag.getMemFrag().ptr + 1) = LAST_CCSRL_CHUNK;
                data->appendMediaFragment(CCSRLFrag);

                newFrag.getMemFrag().len = size - offset;

                offset += (size - offset);
            }
            else
            {
                /* Intermediate CCSRL chunk */
                *((uint8 *)CCSRLFrag.getMemFrag().ptr + 1) = INTERMEDIATE_CCSRL_CHUNK;
                data->appendMediaFragment(CCSRLFrag);

                newFrag.getMemFrag().len = iCcsrlSduSize;

                offset += iCcsrlSduSize;
            }

            data->appendMediaFragment(newFrag);
            if (!CreateMediaData(srpPkt, data))
                return;
            srpPkt->setSeqNum(seqNum);

            /* Queue received PDU */
            SrpSendWaitSet(srpPkt);
        }
    }
    else
    {
        // If no message available to send the entire message
        if (iTxPacketAlloc->NumMsgAvail() == 0)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0x40000020, "SRP::UpperLayerRx - no message available"));
            return;
        }
        if (!Allocate(data, CCSRLFrag))
            return;

        /* Set sequence number */
        seqNum = (uint8) SrpSendSeqGet();
        SrpSendSeqCount();
        *(uint8 *)CCSRLFrag.getMemFrag().ptr = seqNum;
        CCSRLFrag.getMemFrag().len = SRP_SEQUENCE_SIZE;

        data->appendMediaFragment(CCSRLFrag);

        data->appendMediaFragment(frag);

        if (!CreateMediaData(srpPkt, data))
            return;

        //---------------------------------------------------------
        // Send the encoded message (pCodeData, GetSize) to SRP
        //   without using the H.324 multi-chunk method.
        //---------------------------------------------------------

        srpPkt->setSeqNum(seqNum);

        /* Queue received PDU */
        SrpSendWaitSet(srpPkt);
    }

    /* Process call */
    SrpStateChange(SrpStatusGet(), EV_PDU, NULL);

    return;
}

bool SRP::Allocate(OsclSharedPtr<PVMFMediaDataImpl>& data, OsclRefCounterMemFrag& CCSRLFrag)
{
    data = iTxPacketAlloc->allocate();

    if (!data)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0x40000020, "SRP::UpperLayerRx - Unable allocate tx media impl"));
        return false;
    }

    CCSRLFrag = iCcsrlAllocator.get();

    return (CCSRLFrag.getMemFragPtr() ? true : false);
}

/************************************************************************/
/*  function name       : UseNSRP								   		*/
/*  function outline    : Set use of NSRP response frames or not.       */
/*  function description: void UseNSRP( bool aUseNsrp )					*/
/*  input data          : None							                */
/*  output data         : None                                          */
/*  draw time           : '05.08.23                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                          Copyright (C) 2005 PacketVideo Corp.        */
/************************************************************************/
void SRP::UseNSRP(bool aUseNsrp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO, (0x40000020, "SRP::UseNSRP - cur %d, new %d", iUseNSRP, aUseNsrp));
    if (aUseNsrp == iUseNSRP)
    {
        return;
    }
    iUseNSRP = aUseNsrp;
    iFirstNSRPResp = false;
    return;
}

/************************************************************************/
/*  function name       : DisableWNSRPSupport					   		*/
/*  function outline    : Disables all support for WNSRP.  Must be      */
/*						  called before initailization.					*/
/*  function description: void DisableWNSRPSupport()					*/
/*  input data          : None							                */
/*  output data         : None                                          */
/*  draw time           : '05.08.23                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                          Copyright (C) 2005 PacketVideo Corp.        */
/************************************************************************/
void SRP::DisableWNSRPSupport()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0x40000020, "SRP::DisableWNSRPSupport"));

    if (WnsrpStatusGet() == WNSRP_TX_SUPPORT)
    {
        WnsrpStatusSet(NO_WNSRP_SUPPORT);
    }

    iHandleWNSRP = false;
    return;
}

/************************************************************************/
/*  function name       : SrpResetStats									*/
/*  function outline    : Reset statistics                              */
/*  function description: void SrpResetStats( void )					*/
/*  input data          : None							                */
/*  output data         : None                                          */
/*  draw time           : '96.10.29                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void SRP::SrpResetStats()
{
    iStats.Reset();
    iTotalRespTime = 0;
}

/************************************************************************/
/*  function name       : SrpGetStats									*/
/*  function outline    : Retrieve statistics                           */
/*  function description: void SrpGetStats(SRPStats &aStats)			*/
/*  input data          : aStats - iStats object to fill                 */
/*  output data         : None                                          */
/*  draw time           : '96.10.29                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void SRP::SrpGetStats(SRPStats &aStats)
{
    if (iStats.totalCCSRLChunksSent > 0)
    {
        iStats.aveRespTime = iTotalRespTime / iStats.totalCCSRLChunksSent;
    }
    else
    {
        iStats.aveRespTime = 0;
    }

    aStats = iStats;
}

/************************************************************************/
/*  function name       : UpdateRespStats								*/
/*  function outline    : Update response statistics                    */
/*  function description: void UpdateRespStats()						*/
/*  input data          : None							                */
/*  output data         : None                                          */
/*  draw time           : '96.10.29                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                          Copyright (C) 1996 NTT DoCoMo               */
/************************************************************************/
void SRP::UpdateRespStats()
{
    TimeValue timeNow;
    uint32 timeDifference;

    timeNow.set_to_current_time();

    timeDifference = timeNow.to_msec() - iInitialTimeChunkSent.to_msec();

    if (timeDifference > iStats.maxRespTime)
    {
        iStats.maxRespTime = timeDifference;
    }
    else if (timeDifference < iStats.minRespTime)
    {
        iStats.minRespTime = timeDifference;
    }

    iTotalRespTime += timeDifference;
}

/************************************************************************/
/*  function name       : GetFreeWnsrpTimer								*/
/*  function outline    : Get free WNSRP response timer                 */
/*  function description: SRPRespTimer *GetFreeWnsrpTimer()				*/
/*  input data          : None							                */
/*  output data         : None                                          */
/*  draw time           : '05.08.26                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                          Copyright (C) 2005 PacketVideo Corp.        */
/************************************************************************/
SRP::SRPRespTimer *SRP::GetFreeWnsrpTimer()
{
    SRPRespTimer *timer = NULL;
    if (!iFreeWnsrpCommandSave.empty())
    {
        timer = iFreeWnsrpCommandSave[0];
        iFreeWnsrpCommandSave.erase(iFreeWnsrpCommandSave.begin());
    }

    return timer;
}

/************************************************************************/
/*  function name       : FindActiveWnsrpTimer							*/
/*  function outline    : Get free WNSRP response timer                 */
/*  function description: SRPRespTimer *FindActiveWnsrpTimer(			*/
/*														int32 timerID)	*/
/*  input data          : timerID - id of timer			                */
/*  output data         : None                                          */
/*  draw time           : '05.08.26                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                          Copyright (C) 2005 PacketVideo Corp.        */
/************************************************************************/
SRP::SRPRespTimer *SRP::FindActiveWnsrpTimer(int32 timerID)
{
    for (uint32 i = 0; i < iActiveWnsrpCommandSave.size(); i++)
    {
        if (iActiveWnsrpCommandSave[i]->timerID == timerID)
        {
            return iActiveWnsrpCommandSave[i];
        }
    }

    return NULL;
}

/************************************************************************/
/*  function name       : RemoveActiveWnsrpTimer						*/
/*  function outline    : Remove an timer from the active WNSRP timer   */
/*						  list											*/
/*  function description: void RemoveActiveWnsrpTimer(					*/
/*												SRPRespTimer *timer)	*/
/*  input data          : timer - timer to remove		                */
/*  output data         : None                                          */
/*  draw time           : '05.08.26                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                          Copyright (C) 2005 PacketVideo Corp.        */
/************************************************************************/
void SRP::RemoveActiveWnsrpTimer(SRPRespTimer *timer)
{
    //Timer should be stopped before this method is called.
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0x40000020, "SRP::RemoveActiveWnsrpTimer id %d", timer->timerID));

    for (uint32 i = 0; i < iActiveWnsrpCommandSave.size(); i++)
    {
        if (iActiveWnsrpCommandSave[i] == timer)
        {
            FreeWnsrpTimer(iActiveWnsrpCommandSave[i]);
            iActiveWnsrpCommandSave.erase(&iActiveWnsrpCommandSave[i]);

            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0x40000020, "SRP::RemoveActiveWnsrpTimer timer removed index %d", i));
            return;
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0x40000020, "SRP::RemoveActiveWnsrpTimer timer not found"));
    return;
}


/************************************************************************/
/*  function name       : FreeRxFrag									*/
/*  function outline    : Free a frag to the free list					*/
/*  function description: void FreeRxFrag(SRPRxData *frag)				*/
/*  input data          : timer - timer to remove		                */
/*  output data         : None                                          */
/*  draw time           : '05.08.26                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                          Copyright (C) 2005 PacketVideo Corp.        */
/************************************************************************/
void SRP::FreeRxFrag(SRPRxData *frag)
{
    frag->seqNum = 0;
    frag->data.Unbind();

    frag->next = iFreeRxFragList;
    iFreeRxFragList = frag;

    return;
}

/************************************************************************/
/*  function name       : GetRxFrag										*/
/*  function outline    : Remove an frag from the free list				*/
/*  function description: SRPRxData *GetRxFrag()						*/
/*  input data          : timer - timer to remove		                */
/*  output data         : None                                          */
/*  draw time           : '05.08.26                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                          Copyright (C) 2005 PacketVideo Corp.        */
/************************************************************************/
SRP::SRPRxData *SRP::GetRxFrag(void)
{
    SRPRxData *free;
    if (iFreeRxFragList)
    {
        free = iFreeRxFragList;
        iFreeRxFragList = iFreeRxFragList->next;
        return free;
    }
    else
    {
        return NULL;
    }
}

/************************************************************************/
/*  function name       : AddPendingRxFrag								*/
/*  function outline    : Add a rx frag to the pending list in order of */
/*						  rx sequence number mod 256.					*/
/*  function description: void AddPendingRxFrag(SRPRxData *frag)		*/
/*  input data          : timer - timer to remove		                */
/*  output data         : None                                          */
/*  draw time           : '05.08.26                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                          Copyright (C) 2005 PacketVideo Corp.        */
/************************************************************************/
void SRP::AddPendingRxFrag(SRPRxData *frag)
{
    SRPRxData *cur = iPendingRxFragList;
    SRPRxData *prev = NULL;

    while (cur)
    {
        if (cur->seqNum > frag->seqNum)
        {
            if (prev)
            {
                prev->next = frag;
                frag->next = cur;
            }
            else
            {
                frag->next = iPendingRxFragList;
                iPendingRxFragList = frag;
            }
            return;
        }

        prev = cur;
        cur = cur->next;
    }

    frag->next = NULL;
    if (prev)
    {
        prev->next = frag;
    }
    else
    {
        iPendingRxFragList = frag;
    }
    return;
}

/************************************************************************/
/*  function name       : CheckRxSeqNum									*/
/*  function outline    : Check Rx sequence number for against the		*/
/*						  pending rx frags.								*/
/*  function description: bool CheckRxSeqNum(uint32 seqNum,				*/
/*														uint8 header)	*/
/*  input data          : timer - timer to remove		                */
/*  output data         : None                                          */
/*  draw time           : '05.08.26                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                          Copyright (C) 2005 PacketVideo Corp.        */
/************************************************************************/
bool SRP::CheckRxSeqNum(uint32 seqNum, uint8 header)
{
    PVMFSharedMediaDataPtr resp;
    uint32 endSeqNum;

    //If WNSRP command
    switch (header)
    {
        case WNSRP_RESPONSE_HEADER:
            //Check if data is already pending to be sent up
            if (CheckPendingRxList(seqNum))
            {
                /* Create WNSRP response */
                resp = SrpResponseCreate((uint8) seqNum, header);
                if (resp.GetRep() == NULL)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0x40000020, "SRP::CheckRxSeqNum - Unable create WNSRP reponse packet"));
                    /* If unable to create response message. */
                    return false;
                }

                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0x40000020, "SRP::CheckRxSeqNum - lowerlayertx wnsrp resp seq num %d", resp->getSeqNum()));

                /* SRP response send */
                LowerLayerTx(resp);
                return false;
            }

            endSeqNum = (SrpRecvSeqGet() + iWNSRPRxWindow) % 256;
            //If rx window does not wrap
            if (endSeqNum > SrpRecvSeqGet())
            {
                //If can't accept sequence number because of rx window
                if ((seqNum < SrpRecvSeqGet()) ||
                        (seqNum > endSeqNum))
                {
                    if (CheckWNSRPRetrans(seqNum))
                    {
                        /* Create WNSRP response */
                        resp = SrpResponseCreate((uint8) seqNum, header);
                        if (resp.GetRep() == NULL)
                        {
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0x40000020, "SRP::CheckRxSeqNum - Unable create WNSRP reponse packet"));
                            /* If unable to create response message. */
                            return false;
                        }

                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0x40000020, "SRP::CheckRxSeqNum - lowerlayertx wnsrp resp seq num %d", resp->getSeqNum()));

                        /* SRP response send */
                        LowerLayerTx(resp);
                    }
                    return false;
                }

                //Else can accept sequence number
            }
            //Else window wraps around
            else
            {
                //If can accept sequence number
                if ((seqNum >= SrpRecvSeqGet()) ||
                        (seqNum <= endSeqNum))
                {
                    return true;
                }
                //Else can't accept sequence number
                else
                {
                    if (CheckWNSRPRetrans(seqNum))
                    {
                        /* Create WNSRP response */
                        resp = SrpResponseCreate((uint8) seqNum, header);
                        if (resp.GetRep() == NULL)
                        {
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0x40000020, "SRP::CheckRxSeqNum - Unable create WNSRP reponse packet"));
                            /* If unable to create response message. */
                            return false;
                        }

                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0x40000020, "SRP::CheckRxSeqNum - lowerlayertx wnsrp resp seq num %d", resp->getSeqNum()));

                        /* SRP response send */
                        LowerLayerTx(resp);
                    }
                    return false;
                }
            }
            return true;

        case NSRP_RESPONSE_HEADER:
        case SRP_RESPONSE_HEADER:
            if (seqNum != SrpRecvSeqGet())
            {
                if (((seqNum + 1) % 256) == SrpRecvSeqGet())
                {
                    /* Create SRP response */
                    resp = SrpResponseCreate((uint8) seqNum, header);
                    if (resp.GetRep() == NULL)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0x40000020, "SRP::CheckRxSeqNum - Unable create SRP reponse packet"));
                        /* If unable to create response message. */
                        return false;
                    }

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0x40000020, "SRP::CheckRxSeqNum - lowerlayertx srp resp seq num %d", resp->getSeqNum()));

                    /* SRP response send */
                    LowerLayerTx(resp);
                }
                return false;
            }
            return true;

        default:
            return false;
    }
}

/************************************************************************/
/*  function name       : CheckPendingRxList							*/
/*  function outline    : Check pending rx list against sequence number	*/
/*  function description: bool CheckRxSeqNum(uint32 seqNum)				*/
/*  input data          : seqNum - sequence number to check	            */
/*  output data         : None                                          */
/*  draw time           : '05.08.26                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                          Copyright (C) 2005 PacketVideo Corp.        */
/************************************************************************/
bool SRP::CheckPendingRxList(uint32 seqNum)
{
    SRPRxData *data = iPendingRxFragList;
    while (data)
    {
        if (data->seqNum == seqNum)
        {
            return true;
        }
        data = data->next;
    }
    return false;
}

/************************************************************************/
/*  function name       : CheckWNSRPRetrans								*/
/*  function outline    : Check Rx sequence number for WNSRP retrans	*/
/*						  window.										*/
/*  function description: bool CheckWNSRPRetrans(uint32 seqNum)			*/
/*  input data          : timer - timer to remove		                */
/*  output data         : None                                          */
/*  draw time           : '05.08.26                                     */
/*----------------------------------------------------------------------*/
/*  amendent career     :                                               */
/*                          Copyright (C) 2005 PacketVideo Corp.        */
/************************************************************************/
bool SRP::CheckWNSRPRetrans(uint32 seqNum)
{
    //Retrans window does not wrap
    if (iOldestWNSRPRetransSeqNum < SrpRecvSeqGet())
    {
        if ((seqNum < SrpRecvSeqGet()) &&
                (seqNum > iOldestWNSRPRetransSeqNum))
        {
            return true;
        }
    }
    //Retrans window wraps
    else if ((seqNum < SrpRecvSeqGet()) ||
             (seqNum > iOldestWNSRPRetransSeqNum))
    {
        return true;
    }

    return false;
}



