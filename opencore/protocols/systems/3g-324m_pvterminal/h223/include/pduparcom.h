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
#if !defined(PDU_PARCOM_H)
#define PDU_PARCOM_H
#include "oscl_base.h"
#include "oscl_mem.h"
#include "media_fragment.h"
#include "media_packet.h"
#include "golay.h"
#include "oscl_time.h"
#include "oscl_rand.h"
#include "h324utils.h"
#include "h223types.h"

#ifndef PVMF_MEDIA_FRAG_GROUP_H_INCLUDED
#include "pvmf_media_frag_group.h"
#endif

#ifndef PVMF_POOL_BUFFER_ALLOCATOR_H_INCLUDED
#include "pvmf_pool_buffer_allocator.h"
#endif

#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif

typedef enum
{
    EHeaderErr = 0,
    ESizeErr
}EMuxPduError;

class H223PduParcomObserver
{
    public:
        virtual ~H223PduParcomObserver() {}
        virtual uint32 MuxPduIndicate(uint8* pPdu, uint32 pduSz, int32 fClosing, int32 muxCode) = 0;
        virtual void MuxPduErrIndicate(EMuxPduError err) = 0;
        virtual void MuxSetupComplete(PVMFStatus status, TPVH223Level level) = 0;
};

class H223PduParcom
{
    public:
        virtual ~H223PduParcom() {}

        /**
         * Returns the level associated with this parcom.
         **/
        virtual TPVH223Level GetLevel() = 0;

        /**
         * Constructs the parcom based on max_outstanding_pdus.
         **/
        virtual void Construct(uint16 max_outstanding_pdus) = 0;
        /**
         * Sets the base parcom observer.
         **/
        virtual void SetObserver(H223PduParcomObserver* observer) = 0;
        /**
         * Copies one stuffing sequence into the provided buffer.
         * @param buf The buffer to be used.
         * @param buf_size The maximum size of the buffer.
         * @param mux_code The multiplex code to be used in the stuffing header (may not apply for some parcoms)
         * @return The number of bytes copied.
         **/
        virtual uint32 GetStuffing(uint8* buf, uint32 buf_size, uint8 mux_code = 0) = 0;

        /**
         * @return The size of the header field.
         **/
        virtual uint32 GetHeaderSz() = 0;

        /**
         * Returns the stuffing size.
         * @return The size of the stuffing sequence.
         **/
        virtual uint32 GetStuffingSz() = 0;

        /**
         * Parses the bitstream and returns pdus using the MuxPduIndicate observer callback.
         * @param buf The buffer to be used.
         * @param buf_size The maximum size of the buffer.
         * @param mux_code The multiplex code to be used in the stuffing header (may not apply for some parcoms)
         * @return The number of bytes copied.
         **/
        virtual uint32 Parse(uint8* buffer, uint32 buffer_size) = 0;

        /**
         * Returns a header fragment.
         * @param frag The buffer to be used.
         * @param buf_size The maximum size of the buffer.
         * @param mux_code The multiplex code to be used in the stuffing header (may not apply for some parcoms)
         * @return The number of bytes copied.
         **/
        virtual void GetHdrFragment(OsclRefCounterMemFrag& frag) = 0;

        /**
         * Completes a pdu (sets the header and trailer info if any).
         * @param pdu The pdu to be completed.
         * @param mux_code The multiplex code for this pdu.
         * @param packet_marker The packet marker field.
         * @return The return status.
         **/
        virtual PVMFStatus CompletePdu(OsclSharedPtr<PVMFMediaDataImpl>& pdu, int8 mux_code, uint8 packet_marker) = 0;

        virtual void ResetStats() = 0;
        virtual void LogStats(TPVDirection dir) = 0;
};

typedef OsclSharedPtr<H223PduParcom> H223PduParcomSharedPtr;

/** Base implementation with some common features to Level0,1,2. **/
class H223PduParcomBase : public H223PduParcom
{
    public:
        H223PduParcomBase():
                iObserver(NULL),
                iLogger(NULL),
                iPduEndPos(NULL)
        {
            iPduPos = iPdu;
            iPduEndPos = &iPdu[H223_MAX_DEMUX_PDU_SIZE-1];
        }

        void SetObserver(H223PduParcomObserver* observer)
        {
            iObserver = observer;
        }

        void ResetStats();
        void LogStats(TPVDirection dir);
    protected:
        H223PduParcomObserver* iObserver;
        PVLogger* iLogger;

        uint8 iPdu[H223_MAX_DEMUX_PDU_SIZE];
        uint8* iPduPos;
        uint8* iPduEndPos;

        // Outgoing
        uint32 iNumFlagsTx;
        uint32 iNumStuffingTx;
        uint32 iNumPdusTx;

        // Incoming
        uint32 iNumFlagsRx;
        uint32 iNumStuffingRx;
        uint32 iNumPdusRx;
        uint32 iNumPmRx;

        uint32 iNumFlagErrorsRx;
        uint32 iCumFlagErrorsRx;
        uint32 iNumSyncLossRx;
        uint32 iNumBytesLossRx;
        uint32 iNumPduHdrErrorRx;
        uint32 iCumPduHdrErrorRx;
};

class PduParcomRefCounter: public OsclRefCounter
{
    public:
        PduParcomRefCounter(H223PduParcom* p): ptr(p), refcnt(1) {}

        void addRef()
        {
            ++refcnt;
        }

        void removeRef()
        {
            --refcnt;
            if (refcnt == 0)
            {
                if (ptr)
                {
                    delete ptr;
                }
                delete this;
            }
        }

        uint32 getCount()
        {
            return refcnt;
        }
    private:
        H223PduParcom* ptr;
        uint32 refcnt;
};

#endif
