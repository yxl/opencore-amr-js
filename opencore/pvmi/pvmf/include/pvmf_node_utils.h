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

#ifndef PVMF_NODE_UTILS_H_INCLUDED
#define PVMF_NODE_UTILS_H_INCLUDED

#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif
#ifndef OSCL_STRING_CONTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif

/**
//A basic implemention of PVInterface.  Interface implementations
//can derive from this.
*/
template<class Alloc>
class PVInterfaceImpl
{
    public:
        PVInterfaceImpl(const PVUuid& uuid)
                : iRefCounter(1)
                , iUuid(uuid)
        {}
        virtual ~PVInterfaceImpl()
        {}
        void removeRef()
        {
            --iRefCounter;
            if (iRefCounter <= 0)
            {
                this->~PVInterfaceImpl();
                Alloc alloc;
                alloc.deallocate(this);
            }
        }
        void addRef()
        {
            iRefCounter++;
        }
        const PVUuid& Uuid()const
        {
            return iUuid;
        }
    private:
        int32 iRefCounter;
        PVUuid iUuid;
};


/**
//A vector for holding port pointers, with a built-in port iterator.
//This vector automatically calls the port destructor when pointers are
//removed from the vector.
//Node implementations can use this to manage multiple ports.
*/
template<class Port, class Alloc>
class PVMFPortVector: public PVMFPortIter
{
    public:
        typedef Port vec_element_deref;
        typedef Port* vec_element;
        typedef vec_element* vec_element_ptr;

        virtual ~PVMFPortVector()
        {
            while (iVec.size() > 0)
                Erase(&front());
        }
        void Construct(uint32 nres)
        {//construct the vector
            iIterIndex = 0;
            iVec.reserve(nres);
            iNres = nres;
        }
        void Erase(vec_element_ptr elem)
        {//erase an element
            if (elem)
            {	//must explicitly call port destructor, since
                //vec elements are pointers.
                Oscl_TAlloc<vec_element_deref, Alloc> talloc;
                talloc.destruct_and_dealloc(*elem);
                iVec.erase(elem);
            }
        }
        void Reconstruct()
        {//erase all elements & reconstruct the vector.
            for (uint32 i = 0;i < iVec.size();i++)
                Erase(&iVec[i]);
            iVec.clear();
            iVec.reserve(iNres);
        }

        OsclAny* Allocate()
        {//allocate space for a port.
            Alloc alloc;
            return alloc.ALLOCATE(sizeof(Port));
        }
        void DestructAndDealloc(Port* port)
        {//Cleanup a port using the allocator.  For use only
            //on port pointers that aren't yet in the vector.
            Oscl_TAlloc<Port, Alloc> talloc;
            talloc.destruct_and_dealloc(port);
        }

        void AddL(vec_element &elem)
        {//add an element to the end.
            iVec.push_back(elem);
        }

        void InsertL(vec_element &elem)
        {//add an element to the front
            iVec.push_front(elem);
        }

        vec_element_ptr FindByValue(vec_element &elem)
        {//find an element by its value
            for (uint32 i = 0;i < iVec.size();i++)
                if (iVec[i] == elem)
                    return &iVec[i];
            return NULL;
        }

        //From PVMFPortIter
        uint16 NumPorts()
        {
            return (uint16)iVec.size();
        }
        PVMFPortInterface* GetNext()
        {
            if (iVec.size() > iIterIndex)
                return iVec[iIterIndex++];
            return NULL;
        }
        void Reset()
        {
            iIterIndex = 0;
        }

        //wrappers for methods from Oscl_Vector
        vec_element& operator[](uint32 n)
        {
            return iVec[n];
        }
        const vec_element& operator[](uint32 n) const
        {
            return iVec[n];
        }
        uint32 size()const
        {
            return iVec.size();
        }
        void clear()
        {
            iVec.clear();
        }
        vec_element& front()
        {
            return iVec.front();
        }
        bool empty()const
        {
            return iVec.empty();
        }

    private:
        Oscl_Vector<vec_element, Alloc> iVec;
        uint32 iNres;
        uint32 iIterIndex;
};

/**
// Node Command queue utilities.
*/

//IDs for all of the asynchronous node commands.
enum TPVMFGenericNodeCommand
{
    PVMF_GENERIC_NODE_QUERYUUID
    , PVMF_GENERIC_NODE_QUERYINTERFACE
    , PVMF_GENERIC_NODE_REQUESTPORT
    , PVMF_GENERIC_NODE_RELEASEPORT
    , PVMF_GENERIC_NODE_INIT
    , PVMF_GENERIC_NODE_PREPARE
    , PVMF_GENERIC_NODE_START
    , PVMF_GENERIC_NODE_STOP
    , PVMF_GENERIC_NODE_FLUSH
    , PVMF_GENERIC_NODE_PAUSE
    , PVMF_GENERIC_NODE_RESET
    , PVMF_GENERIC_NODE_CANCELALLCOMMANDS
    , PVMF_GENERIC_NODE_CANCELCOMMAND
    , PVMF_GENERIC_NODE_COMMAND_LAST //a placeholder for adding
    //node-specific commands to this list.
};

/**
//A node command class with constructors and destructors
//for all of the generic asynchronous node commands.
*/
template<class Alloc>
class PVMFGenericNodeCommand
{
    public:
        virtual ~PVMFGenericNodeCommand() {}

        //base construction for all commands.  derived class can override this
        //to add initialization of other parameters.
        virtual void BaseConstruct(PVMFSessionId s, int32 aCmd, const OsclAny* aContext)
        {
            iSession = s;
            iCmd = aCmd;
            iContext = aContext;
            iParam1 = NULL;
            iParam2 = NULL;
            iParam3 = NULL;
            iParam4 = NULL;
            iParam5 = NULL;
        }

        //for Init, Start, Stop, Pause and other commands.
        void Construct(PVMFSessionId s, int32 aCmd, const OsclAny* aContext)
        {
            BaseConstruct(s, aCmd, aContext);
        }

        //for CancelCommand
        void Construct(PVMFSessionId s, int32 aCmd, int32 aTag, const OsclAny* aContext)
        {
            BaseConstruct(s, aCmd, aContext);
            iParam1 = (OsclAny*)aTag;
        }
        void Parse(int32&aTag)
        {
            aTag = (int32)iParam1;
        }

        //for ReleasePort
        void Construct(PVMFSessionId s, int32 aCmd, PVMFPortInterface& aPort, const OsclAny* aContext)
        {
            BaseConstruct(s, aCmd, aContext);
            iParam1 = (OsclAny*) & aPort;
        }
        void Parse(PVMFPortInterface*&aPort)
        {
            aPort = (PVMFPortInterface*)iParam1;
        }

        //for QueryInterface
        void Construct(PVMFSessionId s, int32 aCmd, const PVUuid& aUuid,
                       PVInterface*& aInterfacePtr,
                       const OsclAny* aContext)
        {
            BaseConstruct(s, aCmd, aContext);
            //if input cmd id isn't as expected, memory won't get
            //cleaned up later, so assert here.
            OSCL_ASSERT(aCmd == PVMF_GENERIC_NODE_QUERYINTERFACE);
            //allocate a copy of the Uuid.
            Oscl_TAlloc<PVUuid, Alloc>uuid;
            iParam1 = uuid.ALLOC_AND_CONSTRUCT(aUuid);
            iParam2 = (OsclAny*) & aInterfacePtr;
        }
        void Parse(PVUuid*&aUuid, PVInterface**&aInterface)
        {
            aUuid = (PVUuid*)iParam1;
            aInterface = (PVInterface**)iParam2;
        }

        //for QueryUuids
        void Construct(PVMFSessionId s, int32 aCmd, const PvmfMimeString& aMimeType,
                       Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids,
                       bool aExactUuidsOnly,
                       const OsclAny* aContext)
        {
            BaseConstruct(s, aCmd, aContext);
            //if input cmd id isn't as expected, memory won't get
            //cleaned up later, so assert here.
            OSCL_ASSERT(aCmd == PVMF_GENERIC_NODE_QUERYUUID);
            //allocate a copy of the mime type string.
            Oscl_TAlloc<OSCL_HeapString<Alloc>, Alloc> mimetype;
            iParam1 = mimetype.ALLOC_AND_CONSTRUCT(aMimeType);
            iParam2 = (OsclAny*) & aUuids;
            iParam3 = (OsclAny*)aExactUuidsOnly;
        }
        void Parse(OSCL_String*&aMimetype, Oscl_Vector<PVUuid, OsclMemAllocator>*&aUuids, bool &aExact)
        {
            aMimetype = (OSCL_HeapString<Alloc>*)iParam1;
            aUuids = (Oscl_Vector<PVUuid, OsclMemAllocator>*)iParam2;
            aExact = (iParam3) ? true : false;
        }

        //for RequestPort
        void Construct(PVMFSessionId s, int32 aCmd
                       , int32 aPortTag
                       , const PvmfMimeString* aMimeType
                       , const OsclAny* aContext)
        {
            BaseConstruct(s, aCmd, aContext);
            iParam1 = (OsclAny*)aPortTag;
            //if input cmd id isn't as expected, memory won't get
            //cleaned up later, so assert here.
            OSCL_ASSERT(aCmd == PVMF_GENERIC_NODE_REQUESTPORT);
            //allocate a copy of the mime type string.
            if (aMimeType)
            {
                Oscl_TAlloc<OSCL_HeapString<Alloc>, Alloc> mimetype;
                iParam2 = mimetype.ALLOC_AND_CONSTRUCT(*aMimeType);
            }
        }
        void Parse(int32&aPortTag, OSCL_String*&aMimetype)
        {
            aPortTag = (int32)iParam1;
            aMimetype = (OSCL_HeapString<Alloc>*)iParam2;
        }

        //cleanup routine.  Derived class can override this to cleanup additional
        //allocated memory.
        virtual void Destroy()
        {
            switch (iCmd)
            {
                case PVMF_GENERIC_NODE_QUERYUUID:
                {//destroy the allocated mimetype string
                    Oscl_TAlloc<OSCL_HeapString<Alloc>, Alloc> mimetype;
                    mimetype.destruct_and_dealloc(iParam1);
                }
                break;
                case PVMF_GENERIC_NODE_REQUESTPORT:
                {//destroy the allocated mimetype string
                    if (iParam2)
                    {
                        Oscl_TAlloc<OSCL_HeapString<Alloc>, Alloc> mimetype;
                        mimetype.destruct_and_dealloc(iParam2);
                    }
                }
                break;
                case PVMF_GENERIC_NODE_QUERYINTERFACE:
                {//destroy the allocated uuid
                    Oscl_TAlloc<PVUuid, Alloc>uuid;
                    uuid.destruct_and_dealloc(iParam1);
                }
                break;
                default:
                    break;
            }
        }

        //command copy.  derived class can override this to allocate
        //any additional parameters.
        virtual void Copy(const PVMFGenericNodeCommand<Alloc>& aCmd)
        {
            iId = aCmd.iId;
            iSession = aCmd.iSession;
            iContext = aCmd.iContext;
            iParam1 = aCmd.iParam1;
            iParam2 = aCmd.iParam2;
            iParam3 = aCmd.iParam3;
            iParam4 = aCmd.iParam4;
            iParam5 = aCmd.iParam5;
            iCmd = aCmd.iCmd;
            switch (aCmd.iCmd)
            {
                case PVMF_GENERIC_NODE_QUERYUUID:
                {//copy the allocated mimetype string
                    OSCL_HeapString<Alloc>* aMimetype = (OSCL_HeapString<Alloc>*)aCmd.iParam1;
                    Oscl_TAlloc<OSCL_HeapString<Alloc>, Alloc> mimetype;
                    iParam1 = mimetype.ALLOC_AND_CONSTRUCT(*aMimetype);
                }
                break;
                case PVMF_GENERIC_NODE_REQUESTPORT:
                {//copy the allocated mimetype string
                    OSCL_HeapString<Alloc>* aMimetype = (OSCL_HeapString<Alloc>*)aCmd.iParam2;
                    if (aMimetype)
                    {
                        Oscl_TAlloc<OSCL_HeapString<Alloc>, Alloc> mimetype;
                        iParam2 = mimetype.ALLOC_AND_CONSTRUCT(*aMimetype);
                    }
                }
                break;
                case PVMF_GENERIC_NODE_QUERYINTERFACE:
                {//copy the allocated uuid
                    PVUuid* aUuid = (PVUuid*)aCmd.iParam1;
                    Oscl_TAlloc<PVUuid, Alloc>uuid;
                    iParam1 = uuid.ALLOC_AND_CONSTRUCT(*aUuid);
                }
                break;
                default:
                    break;
            }
        }

        //this routine identifies commands that need to
        //go at the front of the queue.  derived command
        //classes can override it if needed.
        virtual bool hipri()
        {
            return (iCmd == PVMF_GENERIC_NODE_CANCELALLCOMMANDS
                    || iCmd == PVMF_GENERIC_NODE_CANCELCOMMAND);
        }

        //allocate space for a command using the class allocator.
        static OsclAny* Allocate()
        {
            Alloc alloc;
            return alloc.ALLOCATE(sizeof(PVMFGenericNodeCommand));
        }

        //command parameters.
        PVMFCommandId iId;
        PVMFSessionId iSession;
        const OsclAny *iContext;
        OsclAny* iParam1;
        OsclAny* iParam2;
        OsclAny* iParam3;
        OsclAny* iParam4;
        OsclAny* iParam5;
        int32 iCmd;
};

/**
//A command queue with a built-in command ID generator.
*/
template<class Command, class Alloc>
class PVMFNodeCommandQueue
{
    public:
        typedef Command vec_element;
        typedef vec_element* vec_element_ptr;

        PVMFNodeCommandQueue()
        {
            iCommandCounter = 0;
        }

        //note: usage of the class requires calling the Construct function
        void Construct(int32 init, uint32 nres)
        {//construct the vector
            iCommandCounter = init;
            iVec.reserve(nres);
        }

        ~PVMFNodeCommandQueue()
        {
            while (!empty())
            {//must explicitly destroy all elements.
                Erase(&iVec[0]);
            }
        }

        void Erase(vec_element_ptr elem)
        {//erase an element
            elem->Destroy();
            iVec.erase(elem);
        }

        int32 AddL(vec_element &elem)
        {//add an element with a new ID
            elem.iId = iCommandCounter++;
            if (elem.hipri())
                iVec.push_front(elem);
            else
                iVec.push_back(elem);
            return elem.iId;
        }

        vec_element_ptr StoreL(vec_element &elem)
        {//store a copy of an element
            vec_element newelem;
            newelem.Copy(elem);
            if (newelem.hipri())
            {
                iVec.push_front(newelem);
                return &iVec[0];
            }
            else
            {
                iVec.push_back(newelem);
                return &iVec[iVec.size()-1];
            }
        }

        vec_element_ptr FindById(PVMFCommandId aId, uint32 aOffset = 0)
        {//find an element by its command id
            for (uint32 i = aOffset;i < iVec.size();i++)
                if (iVec[i].iId == aId)
                    return &iVec[i];
            return NULL;
        }

        //Wrappers for methods from Oscl_Vector
        vec_element& operator[](uint32 n)
        {
            return iVec[n];
        }
        const vec_element& operator[](uint32 n) const
        {
            return iVec[n];
        }
        uint32 size()const
        {
            return iVec.size();
        }
        void clear()
        {
            iVec.clear();
        }
        vec_element& front()
        {
            return iVec.front();
        }
        bool empty()const
        {
            return iVec.empty();
        }

    private:
        Oscl_Vector<vec_element, Alloc> iVec;
        int32 iCommandCounter;
};




#endif


