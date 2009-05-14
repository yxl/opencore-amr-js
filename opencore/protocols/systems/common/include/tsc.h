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
#if !defined (TSC_H_INCLUDED)
#define TSC_H_INCLUDED

#include "pvt_params.h"

#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif

#define PV_2WAY_TSC_EXTENSIONINTERFACE_UUID PVUuid(0x50e23520,0xf8a3,0x11d9,0xbe,0xab,0x00,0x02,0xa5,0xd5,0xc5,0x1b)
enum TSCState
{
    TSC_Idle = 0,
    TSC_Connecting,
    TSC_Communicating,
    TSC_Disconnected
};

/** Observer interface for TSC extension interface **/
class TSCObserver
{
    public:
        virtual ~TSCObserver() {}
        /* Responses to commands */
        /* Indicates completion of a previously issued Connect command */
        virtual void ConnectComplete(PVMFStatus status) = 0;

        /* Unsolicited indications */
        /* An internal error has occurred.  User should disconnect and teardown */
        virtual void InternalError() = 0;
        /* Requests the user to stop av codecs.  Logical channels will be closed by TSC */
        virtual void DisconnectRequestReceived() = 0;
        /* Indicates establishment of an outgoing logical channel by the stack */
        virtual void OutgoingChannelEstablished(TPVChannelId aId,
                                                PVCodecType_t aCodec,
                                                uint8* aFormatSpecificInfo = NULL, uint32 aFormatSpecificInfoLen = 0) = 0;
        /* Indicates establishment of an incoming logical channel by the stack */
        virtual TPVStatusCode IncomingChannel(TPVChannelId aId,
                                              PVCodecType_t aCodec,
                                              uint8* aFormatSpecificInfo = NULL, uint32 aFormatSpecificInfoLen = 0) = 0;
        /* Indicates closure of a logical channel by the stack */
        virtual void ChannelClosed(TPVDirection direction,
                                   TPVChannelId id,
                                   PVCodecType_t codec,
                                   PVMFStatus status = PVMFSuccess) = 0;
        /* Requests the user to generate Intra content for the specified port/logical channel */
        virtual void RequestFrameUpdate(PVMFPortInterface *port) = 0;
};

#define MAX_STACK_ELEMENTS 10

/** Abstract extension interface for all 2-way Terminal State Controllers **/
class TSC : public PVInterface
{
    public:
        virtual ~TSC() {};
        /* Terminal wide commands */
        /* Initializes the TSC with reference to mux and controls */
        virtual TPVStatusCode InitTsc() = 0;
        virtual TSCObserver* SetTscObserver(TSCObserver* aObserver)
        {
            if (!iObserver)
            {
                iObserver = aObserver;
                return iObserver;
            }
            return NULL;
        }
        //virtual TPVStatusCode SetCapability(CapabilitySet* capabilities) = 0;
        virtual CPvtTerminalCapability* GetRemoteCapability() = 0;
        virtual TSCState GetTscState()
        {
            return iState;
        }
        virtual TPVStatusCode ResetTsc() = 0;
        virtual TPVStatusCode Connect(uint16 info_len = 0, uint8* info_buf = NULL) = 0;
        virtual TPVStatusCode SetTimerRes(uint32 timer_res) = 0;

        virtual TPVStatusCode Disconnect() = 0;
        virtual TPVStatusCode Abort() = 0;

        /* Channel specific commands */
        virtual TPVStatusCode SetTerminalParam(CPVTerminalParam* params) = 0;
        virtual CPVTerminalParam* GetTerminalParam() = 0;
        virtual TPVStatusCode SetOutgoingBitrate(int32 bitrate) = 0;
        virtual TPVStatusCode RequestFrameUpdate(PVMFPortInterface* port) = 0;
        /* Returns a pointer to the logical channels buffer.  DO NOT DELETE */
        virtual const uint8* GetFormatSpecificInfo(PVMFPortInterface* port, uint32* len) = 0;

        virtual void SetLoopbackMode(TPVLoopbackMode aLoopbackMode) = 0;
        virtual void ResetStats() = 0;
        virtual void LogStats(TPVDirection dir) = 0;
        virtual LogicalChannelInfo* GetLogicalChannelInfo(PVMFPortInterface& port) = 0;
        virtual void SetDatapathLatency(TPVDirection aDir, PVMFPortInterface* aPort, uint32 aLatency) = 0;
        virtual void SetSkewReference(PVMFPortInterface* aPort, PVMFPortInterface* aReferencePort) = 0;

    protected:
        TSC() : iState(TSC_Idle), iObserver(NULL)
        {
        }

        TSCState iState;
        TSCObserver* iObserver;
    private:
};

#endif

