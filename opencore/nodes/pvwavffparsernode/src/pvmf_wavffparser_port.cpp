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
#include "pvmf_wavffparser_port.h"
#include "pvmf_wavffparser_node.h"
#include "pvmf_wavffparser_defs.h"


PVMFWAVFFParserOutPort::PVMFWAVFFParserOutPort(int32 aTag, PVMFWAVFFParserNode* aNode)
        : PvmfPortBaseImpl(aTag, aNode, "WavFFParOut(Audio)")
{
    //Construct();
    iLogger = PVLogger::GetLoggerObject("PVMFWAVFFParserOutPort");
    oscl_memset(&iStats, 0, sizeof(PvmfPortBaseImplStats));
    iNumFramesGenerated = 0;
    iNumFramesConsumed = 0;
    PvmiCapabilityAndConfigPortFormatImpl::Construct(
        PVMF_WAVFFPARSER_PORT_OUTPUT_FORMATS,
        PVMF_WAVFFPARSER_PORT_OUTPUT_FORMATS_VALTYPE);
    iNode = aNode;
}


PVMFWAVFFParserOutPort::~PVMFWAVFFParserOutPort()
{
    Disconnect();
    ClearMsgQueues();
}


bool PVMFWAVFFParserOutPort::IsFormatSupported(PVMFFormatType aFmt)
{
    if ((aFmt == PVMF_MIME_PCM ||
            aFmt == PVMF_MIME_PCM8 ||
            aFmt == PVMF_MIME_PCM16 ||
            aFmt == PVMF_MIME_PCM16_BE ||
            aFmt == PVMF_MIME_ULAW ||
            aFmt == PVMF_MIME_ALAW))
    {
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////
void PVMFWAVFFParserOutPort::FormatUpdated()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO
                    , (0, "PVMFWAVFFParserOutPort::FormatUpdated %s", iFormat.getMIMEStrPtr()));
}


#define LOG_ERR(x) PVLOGGER_LOGMSG( PVLOGMSG_INST_MLDBG,iLogger,PVLOGMSG_STACK_TRACE,x);

////////////////////////////////////////////////////////////////////////////

PVMFStatus PVMFWAVFFParserOutPort::Connect(PVMFPortInterface* aPort)
{

    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_STACK_TRACE
                    , (0, "PVMFWAVFFParserOutPort::FormatUpdated %s", iFormat.getMIMEStrPtr()));

    if (!aPort)
    {
        LOG_ERR((0, "PVMFWAVFFParserOutPort::Connect: Error - Connecting to invalid port"));
        return PVMFErrArgument;
    }

    if (iConnectedPort)
    {
        LOG_ERR((0, "PVMFWAVFFParserOutPort::Connect: Error - Already connected"));
        return PVMFFailure;
    }

    OsclAny* temp = NULL;
    aPort->QueryInterface(PVMI_CAPABILITY_AND_CONFIG_PVUUID, temp);
    PvmiCapabilityAndConfig *config = OSCL_STATIC_CAST(PvmiCapabilityAndConfig*, temp);

    if (!config)
    {
        LOG_ERR((0, "PVMFWAVFFParserOutPort::Connect: Error - Peer port does not support capability interface"));
        return PVMFFailure;
    }

    PVMFStatus status = PVMFSuccess;

    status = iNode->NegotiateSettings(config);

    if (status != PVMFSuccess)
    {
        LOG_ERR((0, "PVMFWAVFFParserOutPort::Connect: Error - Settings negotiation failed. status=%d", status));
        return status;
    }

    //Automatically connect the peer.
    if (aPort->PeerConnect(this) != PVMFSuccess)
    {
        LOG_ERR((0, "PVMFWAVFFParserOutPort::Connect: Error - Peer Connect failed"));
        return PVMFFailure;
    }

    iConnectedPort = aPort;

#if PVMF_PORT_BASE_IMPL_STATS
    // Reset statistics
    oscl_memset((OsclAny*)&(PvmfPortBaseImpl::iStats), 0, sizeof(PvmfPortBaseImplStats));
#endif

    PortActivity(PVMF_PORT_ACTIVITY_CONNECT);
    return PVMFSuccess;

}

