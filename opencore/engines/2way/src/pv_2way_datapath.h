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
#ifndef PV_2WAY_DATAPATH_H_INCLUDED
#define PV_2WAY_DATAPATH_H_INCLUDED

#ifndef OSCL_ERROR_H_INCLUDED
#include "oscl_error.h"
#endif

#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif

#ifndef PV_2WAY_ENGINE_H_INCLUDED
#include "pv_2way_engine.h"
#endif

#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif

#ifndef PVMF_PORT_INTERFACE_H_INCLUDED
#include "pvmf_port_interface.h"
#endif

#define MAX_DATAPATH_NODES 10
#define MAX_PARENT_PATHS 2
#define MAX_DEPENDENT_PATHS 2

/**
 * TPVNodeConfigTimeType enum
 *
 * An enumeration of when to configure the node
 **/
typedef enum
{
    EConfigBeforeInit = 0, //Configure node before initialization
    EConfigBeforeReqInPort, //Configure node before requesting input port
    EConfigBeforeReqOutPort, //Configure node before requesting output port
    EConfigBeforePrepare, //Configure node before preparing
    EConfigBeforeStart, //Configure node before starting
    EMaxConfigTimeTypes
} TPVNodeConfigTimeType;

/**
 * TPV2WayDatapathType enum
 *
 * An enumeration of types of data paths
 **/
typedef enum
{
    EUnknownDatapath = 0,
    EEncodeDatapath,
    EDecodeDatapath,
    EMuxDatapath,
    ERecDatapath,
    EPreviewDatapath,
    EMaxDatapathType
} TPV2WayDatapathType;

/**
 * TPV2WayDatapathState enum
 *
 * An enumeration of broad external states of the data path
 **/
typedef enum
{
    EClosed = 0,
    EOpening,
    EOpened,
    EPausing,
    EPaused,
    EUnpausing,
    EClosing,
    EMaxDataPathStates
}TPV2WayDatapathState;

/**
 * TPVPortFormatSetType enum
 *
 * An enumeration of how the format type of the port is determined
 **/
typedef enum
{
    EUserDefined = 0, //User defines format type for port
    EConnectedPortFormat, //Format is set to the connecting port's format
    EUseOtherNodePortFormat, //Format is set to whatever the other node port is set to (nodes in a datapath have only 2 ports)
    EAppDefined, //Format is pre-set during RequestPort.  Does not need to be set
    EMaxFormatSetTypes
} TPVPortFormatSetType;


/**
 * CPV2WayPortPair Class
 *
 * Specifies a pair of node ports that describe a data connection and the status of the ports.
 **/
class CPV2WayPortPair
{
    public:
        CPV2WayPortPair() : iIsConnected(false) {};

        ~CPV2WayPortPair() {};

        bool Connect();
        bool Disconnect();

        CPV2WayPort iSrcPort;
        CPV2WayPort iDestPort;
        bool iIsConnected;
};

/**
 * CPVDatapathPort Class
 *
 * Describes how to use a port in a datatpath.
 **/
class CPVDatapathPort
{
    public:
        CPVDatapathPort() : iPortTag(0),
                iRequestPortState(EPVMFNodeInitialized),
                iCanCancelPort(false),
                iPortSetType(EUserDefined),
                iFormatType(PVMF_MIME_FORMAT_UNKNOWN),
                iDefaultFormatType(PVMF_MIME_FORMAT_UNKNOWN),
                iPortPair(NULL)
        {
        };

        CPVDatapathPort(const CPVDatapathPort &aPort) : iPortTag(aPort.iPortTag),
                iRequestPortState(aPort.iRequestPortState),
                iRequestPortAnytime(aPort.iRequestPortAnytime),
                iCanCancelPort(aPort.iCanCancelPort),
                iPortSetType(aPort.iPortSetType),
                iFormatType(aPort.iFormatType),
                iDefaultFormatType(aPort.iDefaultFormatType),

                iPortPair(aPort.iPortPair)
        {
        };

        ~CPVDatapathPort() {};

        CPVDatapathPort& operator=(const CPVDatapathPort& a);

        int32 iPortTag; // The port tag
        TPVMFNodeInterfaceState iRequestPortState; //State of node to request/release port
        bool iRequestPortAnytime; //Can port be requested in any node state
        bool iCanCancelPort; // Can the request port command be cancelled for the port
        TPVPortFormatSetType iPortSetType; // How to determine format type for the port
        PVMFFormatType iFormatType; // Sets format type for the port
        PVMFFormatType iDefaultFormatType; // In the case the CapConfig interface is stupid
        CPV2WayPortPair *iPortPair; // The out port will be the iSrcPort of the port pair
        // The in port will be the iDestPort of the port pair
};

/**
 * CPVDatapathNode Class
 *
 * Describes how a node is used in a datatpath.
 **/
class CPVDatapathNode
{
    public:
        CPVDatapathNode() : iConfigure(NULL),
                iConfigTime(EConfigBeforeInit),
                iCanNodePause(false),
                iLoggoffOnReset(false),
                iOriginalState(EPVMFNodeIdle),
                iDatapathCloseState(EPVMFNodeLastState),
                iCommandIssued(false)
        {};

        CPVDatapathNode(const CPVDatapathNode &aNode) : iNode(aNode.iNode),
                iConfigure(aNode.iConfigure),
                iConfigTime(aNode.iConfigTime),
                iCanNodePause(aNode.iCanNodePause),
                iLoggoffOnReset(aNode.iLoggoffOnReset),
                iIgnoreNodeState(aNode.iIgnoreNodeState),
                iOriginalState(aNode.iOriginalState),
                iDatapathCloseState(aNode.iDatapathCloseState),
                iInputPort(aNode.iInputPort),
                iOutputPort(aNode.iOutputPort),
                iCommandIssued(aNode.iCommandIssued)
        {};


        ~CPVDatapathNode() {};

        CPVDatapathNode& operator=(const CPVDatapathNode& a);

        bool CloseableState()
        {
            PVMFNodeInterface * nodeIFace = (PVMFNodeInterface *)iNode;
            if (iDatapathCloseState != EPVMFNodeLastState &&
                    nodeIFace->GetState() == iDatapathCloseState)
                return true;
            if (iDatapathCloseState == EPVMFNodeLastState &&
                    nodeIFace->GetState() == iOriginalState)
                return true;
            return false;
        }

        TPV2WayNode iNode; // Actual node
        CPV2WayNodeConfigurationObserver *iConfigure; // Node configuration observer, called when config time is reached
        TPVNodeConfigTimeType iConfigTime; // When to configure node
        bool iCanNodePause; // Is the node capable of pausing
        bool iLoggoffOnReset; // Does node need to logoff after datapath is reset
        bool iIgnoreNodeState; // Flag for that specifies whether the datapath changes the state of the node
        TPVMFNodeInterfaceState iOriginalState; // The state of the node when it is added to the datapath
        TPVMFNodeInterfaceState iDatapathCloseState; // The state in which the node should be in before the
        // datapath closes
        CPVDatapathPort iInputPort;
        CPVDatapathPort iOutputPort;
        bool iCommandIssued;
};


class CPV2WayDatapath : public HeapBase, public CPV2WayNodeCommandObserver
{
    private:
        TPV2WayNode* iNode;
    public:
        CPV2WayDatapath(PVLogger *aLogger,
                        TPV2WayDatapathType aType,
                        PVMFFormatType aFormat,
                        CPV324m2Way *a2Way) : iType(aType),
                iState(EClosed),
                iStateBeforeClose(EClosed),
                iFormat(aFormat),
                iSourceSinkFormat(PVMF_MIME_FORMAT_UNKNOWN),
                i2Way(a2Way),
                iLogger(aLogger),
                iAllPortsConnected(false)
        {};

        virtual ~CPV2WayDatapath() {};

        bool IsPortInDatapath(PVMFPortInterface *aPort);
        bool IsNodeInDatapath(PVMFNodeInterface *aNode);
        bool ResetDatapath();
        bool AddNode(const CPVDatapathNode &aNode);
        bool Open();
        bool Close();
        bool Pause();
        bool Resume();

        bool AddParentDatapath(CPV2WayDatapath &aDatapath);
        bool AddDependentDatapath(CPV2WayDatapath &aDatapath);

        TPV2WayDatapathState GetState()
        {
            return iState;
        }
        void SetFormat(PVMFFormatType aFormatType)
        {
            iFormat = aFormatType;
        }
        PVMFFormatType GetFormat()
        {
            return iFormat;
        }

        void CommandHandler(PV2WayNodeCmdType aType, const PVMFCmdResp& aResponse);

        void CheckPath();
        void SetFormatSpecificInfo(uint8* fsi, uint16 fsi_len);
        uint8* GetFormatSpecificInfo(uint32* len);
        void SetSourceSinkFormat(PVMFFormatType aFormatType);
        PVMFFormatType GetSourceSinkFormat() const;
    protected:
        void ConstructL();

        bool SendNodeCmd(PV2WayNodeCmdType cmd, int i);
        int SetParametersSync(PvmiCapabilityAndConfig * configPtr,
                              PvmiKvp* portParams,
                              PvmiKvp*& portParamsReturn);
        bool CheckNodePorts(bool& aCheckPort, int i);

        void SetPort(CPV2WayPort &aDatapathPort, PVMFPortInterface *aPort)
        {
            aDatapathPort.SetPort(aPort);
        }

        void SetState(TPV2WayDatapathState aState);

        bool IsDatapathNodeClosed(CPVDatapathNode &aNode);
        bool CheckNodePortsL(CPVDatapathNode &aNode);
        PVMFStatus PortStatusChange(PVMFNodeInterface *aNode, PVMFCommandId aId, PVMFPortInterface *aPort);
        CPV2WayPort *RetrievePort(PVMFNodeInterface *aNode, PVMFCommandId aId);
        PVMFStatus ReleaseNodePorts(CPVDatapathNode &aNode);
        void CloseNodePorts(CPVDatapathNode &aNode);
        PVMFCommandId SendNodeCmdL(PV2WayNodeCmdType aCmd,
                                   CPVDatapathNode &aNode,
                                   void *aParam = NULL);

        PVMFStatus CheckConfig(TPVNodeConfigTimeType aConfigTime, CPVDatapathNode &aNode);

        //Can be overriden in derived datapaths
        virtual void CheckOpen();
        virtual void CheckPause();
        virtual void CheckResume();
        virtual void CheckClosed();

        virtual void OpenComplete() = 0;
        virtual void PauseComplete() = 0;
        virtual void ResumeComplete() = 0;
        virtual void CloseComplete() = 0;

        virtual void DatapathError() = 0;

        virtual bool CheckPathSpecificOpen()
        {
            return true;
        }
        virtual bool PathSpecificClose()
        {
            return true;
        }
        virtual bool CheckPathSpecificStart()
        {
            return true;
        }

        virtual void DependentPathClosed(CPV2WayDatapath *aDependentPath);
        bool HaveAllDependentPathsClosed();
        bool IsParentClosing();
        void NotifyParentPaths();
        virtual bool ParentIsClosing();
        virtual bool SingleNodeOpen()
        {
            return false;
        }

        PVMFStatus GetKvp(PVMFPortInterface &aPort, bool aInput,
                          PvmiKvp*& aKvp, int& aNumKvpElem, OsclAny*& aconfigPtr);
        PVMFFormatType GetPortFormatType(PVMFPortInterface& aPort,
                                         bool aInput, PVMFPortInterface* aOtherPort);

        TPV2WayDatapathType iType;
        TPV2WayDatapathState iState;
        TPV2WayDatapathState iStateBeforeClose;
        // The codec type associated with this datapath
        PVMFFormatType iFormat;
        // The format type of the terminating source/sink
        PVMFFormatType iSourceSinkFormat;

        CPV324m2Way *i2Way;
        PVLogger *iLogger;
        bool iAllPortsConnected;

        Oscl_Vector<CPVDatapathNode, BasicAlloc> iNodeList;
        Oscl_Vector<CPV2WayPortPair, BasicAlloc> iPortPairList;

        //List of datapaths this datapath depends on
        Oscl_Vector<CPV2WayDatapath *, BasicAlloc> iParentPathList;
        //List of datapaths that depend on this datapath
        Oscl_Vector<CPV2WayDatapath *, BasicAlloc> iDependentPathList;
        // Format specific info associated with this datapath
        uint8* iFsi;
        uint32 iFsiLen;
};

#endif //PV_2WAY_DATAPATH_H_INCLUDED


