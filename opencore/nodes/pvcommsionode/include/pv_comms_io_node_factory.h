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
#ifndef PV_COMMS_IO_NODE_FACTORY_H_INCLUDED
#define PV_COMMS_IO_NODE_FACTORY_H_INCLUDED

// Forward declaration
class PVMFNodeInterface;
class PvmiMIOControl;
class PVMFNodeCmdStatusObserver;
class PVMFNodeInfoEventObserver;
class PVMFNodeErrorEventObserver;

#define PVCommsIONodeUuid PVUuid(0x860c7c50,0xb76a,0x4672,0x97,0x84,0xba,0x4d,0xce,0xc4,0x3a,0x3e)

/**
 * PVCommsIONodeFactory Class
 *
 * PVCommsIONodeFactory class is a singleton class which instantiates and provides
 * access to PVCommsIONode node. It returns a PVMFNodeInterface reference, the
 * interface class of the PVCommsIONode.
 *
 * The client is expected to contain and maintain a pointer to the instance created
 * while the node is active.
 */
class PVCommsIONodeFactory
{
    public:
        /**
         * Creates an instance of a PVCommsIONode from one or more unidirectional
         * media IO components. If the creation fails, this function will leave.
         *
         * @param aMIOInputControl Media input object to be controlled by this node
         *        this parameter can be NULL of the aMIOOutputControl parameter is
         *        non-NULL
         * @param aMIOOutputControl Media output object to be controlled by this node
         *        this parameter can be NULL of the aMIOInputControl parameter is
         *        non-NULL
         * @returns A pointer to an author or leaves if instantiation fails
         */
        OSCL_IMPORT_REF static PVMFNodeInterface* Create(PvmiMIOControl* aMIOInputControl, PvmiMIOControl *aMIOOutputControl, bool logBitstreams = false);

        /**
         * Creates an instance of a PVCommsIONode from a single bidirectional
         * media IO component. If the creation fails, this function will leave.
         *
         * @param aMIOIControl Media input object to be controlled by this node
         * @returns A pointer to an author or leaves if instantiation fails
         */
        OSCL_IMPORT_REF static PVMFNodeInterface* Create(PvmiMIOControl* aMIOControl, bool logBitstreams = false);

        /**
         * This function allows the application to delete an instance of a PVCommsIONode
         * and reclaim all allocated resources.  An instance can be deleted only in
         * the idle state. An attempt to delete in any other state will fail and return false.
         *
         * @param aNode The PVCommsIONode to be deleted.
         */
        OSCL_IMPORT_REF static void Delete(PVMFNodeInterface* aNode);

        /**
         * This function allows the application to retrieve the
         * media I/O control instance contained within the node.
         *
         * @param aNode The PVCommsIONode
         * @returns Pointer to the media I/O component.
         */
        OSCL_IMPORT_REF static PvmiMIOControl* GetMediaIOControl(PVMFNodeInterface* aNode);
};

#endif // PV_COMMS_IO_NODE_FACTORY_H_INCLUDED
