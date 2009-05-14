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

//               P V M F _ N O D E _ R E G I S T R Y

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

/*! \addtogroup pvmfnoderegistry PVMFNodeRegistry
*
* @{
*/


/*! \file pvmf_node_registry.h
\brief This file contains the declarations for the node registry.
*/

#ifndef PVMF_NODE_REGSITRY_H_INCLUDED
#define PVMF_NODE_REGSITRY_H_INCLUDED

#include "pv_interface.h"
#include "pv_uuid.h"
#include "pvmf_event_handling.h"
#include "oscl_vector.h"
#include "pvmf_format_type.h"
#include "oscl_string_containers.h"

class PVMFNodeInterface;
class PvmiMIOControl;


/*
** An abstract interface class containing a function
** to do a synchronous interface query.
** Each registered component must support this interface.
*/
typedef PVInterface PVMFSimpleQueryInterface ;


/*
** An abstract interface class containing methods
** to create and release a PVMF node.
*/
class PVMFNodeCreationInterface
{
    public:
        /** Virtual destructor
        **   need to be defined to avoid compiler warnings
        **/
        virtual ~PVMFNodeCreationInterface() {}

        /**
        ** Create a node instance.  May throw an exception if
        **  node creation fails due to out-of-memory condition or other error.
        **
        ** @param aParam optional opaque data to be passed to node constructor.
        ** @return pointer to the node.
        */
        virtual PVMFNodeInterface* CreateNode(OsclAny* aParam = NULL) = 0;

        /**
        ** Relese a node instance previously created with CreateNode.
        **
        ** @param aNode (input): the node instance.
        */
        virtual void ReleaseNode(PVMFNodeInterface* aNode) = 0;
};

/*
** An abstract interface class containing methods
** to create and release a Media I/O component.
*/
class PVMFMediaIOCreationInterface
{
    public:
        /** Virtual destructor
        **   need to be defined to avoid compiler warnings
        **/
        virtual ~PVMFMediaIOCreationInterface() {}
        /**
        ** Create a Media I/O component instance.  May throw an exception if
        **  creation fails due to out-of-memory condition or other error.
        **
        ** @param aParam: optional opaque data to pass the MIO constructor.
        ** @return pointer to the component.
        */
        virtual PvmiMIOControl* CreateMediaIO(OsclAny*aParam = NULL) = 0;

        /**
        ** Relese a Media I/O component instance previously created with CreateMediaIO.
        **
        ** @param aMediaIO (input): the media I/O instance.
        */
        virtual void ReleaseMediaIO(PvmiMIOControl* aMediaIO) = 0;
};



/*
** A class to encapsulate PVMF media format information.
** The media format is a MIME string, such as audio/compressed/amr
*/
class PVMFComponentFormatType
{
    public:
        OSCL_IMPORT_REF PVMFComponentFormatType(const char*aFmtStr);

        PVMFComponentFormatType(const PVMFComponentFormatType& s)
        {
            iFormatType = s.iFormatType;
            iFormatString = s.iFormatString;
        }

        OSCL_HeapString<OsclMemAllocator> iFormatString;
        PVMFFormatType iFormatType;

        OSCL_IMPORT_REF bool operator== (const PVMFComponentFormatType& src) const;

};


/*
** A class to encapsulate PVMF media format pair information.
** A format pair consists of an input format and an output format.
*/
class PVMFComponentFormatPairType
{
    public:
        PVMFComponentFormatPairType(char* input
                                    , char* output)
                : iInputFormat(PVMFComponentFormatType(input))
                , iOutputFormat(PVMFComponentFormatType(output))
        {}
        PVMFComponentFormatPairType(PVMFComponentFormatType& input
                                    , PVMFComponentFormatType& output)
                : iInputFormat(input)
                , iOutputFormat(output)
        {}

        PVMFComponentFormatPairType(const PVMFComponentFormatPairType& s)
                : iInputFormat(s.iInputFormat)
                , iOutputFormat(s.iOutputFormat)
        {}

        PVMFComponentFormatType iInputFormat;
        PVMFComponentFormatType iOutputFormat;
};

/*
** Format list types.
*/
typedef Oscl_Vector<PVMFComponentFormatType, OsclMemAllocator> PVMFComponentFormatListType;
typedef Oscl_Vector<PVMFComponentFormatPairType, OsclMemAllocator> PVMFComponentFormatPairListType;

/*
** A class containing information about the capability of a component.
*/
class PVMFComponentCapability
{
    public:
        PVMFComponentCapability()
        {}

        PVMFComponentCapability(const PVMFComponentCapability& s)
                : iInputFormatCapability(s.iInputFormatCapability)
                , iOutputFormatCapability(s.iOutputFormatCapability)
                , iInputOutputFormatCapability(s.iInputOutputFormatCapability)
        {}

        /**
        ** Supported format information can be specified by lists of input
        ** and/or output format, or a list of input/output format pairs.
        */
        void AddInputFormat(const char* fmt)
        {
            iInputFormatCapability.push_back(PVMFComponentFormatType(fmt));
        }
        void AddOutputFormat(char* fmt)
        {
            iOutputFormatCapability.push_back(PVMFComponentFormatType(fmt));
        }
        void AddFormatPair(char* infmt, char* outfmt)
        {
            iInputOutputFormatCapability.push_back(PVMFComponentFormatPairType(infmt, outfmt));
        }

        Oscl_Vector<PVMFComponentFormatType, OsclMemAllocator> iInputFormatCapability;
        Oscl_Vector<PVMFComponentFormatType, OsclMemAllocator> iOutputFormatCapability;
        Oscl_Vector<PVMFComponentFormatPairType, OsclMemAllocator> iInputOutputFormatCapability;
};

/*
** Base class for component registration info.
*/
class PVMFComponentRegistrationInfo
{
    public:
        PVMFComponentRegistrationInfo()
        {
            iHasHardwareAssist = false;
            iQueryInterface = NULL;
        }

        PVMFComponentRegistrationInfo(const PVMFComponentRegistrationInfo& a)
                : iUuid(a.iUuid)
                , iMediaCategory(a.iMediaCategory)
                , iComponentType(a.iComponentType)
                , iCapability(a.iCapability)
                , iHasHardwareAssist(a.iHasHardwareAssist)
                , iQueryInterface(a.iQueryInterface)
        {
        }

        PVMFComponentRegistrationInfo(PVUuid& aUuid
                                      , char* aMediaCategory
                                      , char* aComponentType
                                      , PVMFComponentCapability& aCapability
                                      , bool aHasHardwareAssist
                                      , PVMFSimpleQueryInterface* aQueryInterface)
                : iCapability(aCapability)
        {
            OSCL_UNUSED_ARG(aUuid);

            if (aMediaCategory)
                iMediaCategory = aMediaCategory;
            if (aComponentType)
                iComponentType = aComponentType;
            iHasHardwareAssist = aHasHardwareAssist;
            iQueryInterface = aQueryInterface;
        }


        /**
        ** Unique identifier
        */
        PVUuid iUuid;

        /**
        ** Media category, e.g. "audio", "video", "text", "image", "multi"
        */
        OSCL_HeapString<OsclMemAllocator> iMediaCategory;

        /**
        ** Hierarchical component type string, e.g. "source/capture", "source/file",
        **   "source/media_io",
        **   "file_parser/mpeg4", "codec/audio/aac", "codec/video/h263",
        **   "sink/file", "sink/render", "sink/media_io"
        */
        OSCL_HeapString<OsclMemAllocator> iComponentType;

        /**
        ** Component capability information
        */
        PVMFComponentCapability iCapability;

        /**
        ** Hardware assist flag.  Set to true if component may have hardware assist;
        **   set to false if component is pure software.
        */
        bool iHasHardwareAssist;

        /**
        ** Optional pointer to a simple query interface implementation for the component.
        */
        PVMFSimpleQueryInterface* iQueryInterface;

};

/*
** A class containing all data required to register a node.
** The node may be registered with either an implemenation instance,
** or a creation interface instance.
*/
class PVMFNodeRegistrationInfo: public PVMFComponentRegistrationInfo
{
    public:
        PVMFNodeRegistrationInfo()
        {
            iNodeInstance = NULL;
            iNodeCreationInterface = NULL;
        }
        PVMFNodeRegistrationInfo(const PVMFNodeRegistrationInfo& a)
                : PVMFComponentRegistrationInfo(a)
                , iNodeCreationInterface(a.iNodeCreationInterface)
                , iNodeInstance(a.iNodeInstance)
        {}

        PVMFNodeRegistrationInfo(PVUuid& aUuid
                                 , char* aMediaCategory
                                 , char* aNodeType
                                 , PVMFComponentCapability& aCapability
                                 , bool aHasHardwareAssist
                                 , PVMFSimpleQueryInterface* aQueryInterface
                                 , PVMFNodeInterface& aNodeInstance)
                : PVMFComponentRegistrationInfo(aUuid, aMediaCategory, aNodeType, aCapability, aHasHardwareAssist, aQueryInterface)
                , iNodeInstance(&aNodeInstance)
        {
            iNodeCreationInterface = NULL;
        }

        PVMFNodeRegistrationInfo(PVUuid& aUuid
                                 , char* aMediaCategory
                                 , char* aNodeType
                                 , PVMFComponentCapability& aCapability
                                 , bool aHasHardwareAssist
                                 , PVMFSimpleQueryInterface* aQueryInterface
                                 , PVMFNodeCreationInterface& aNodeCreation)
                : PVMFComponentRegistrationInfo(aUuid, aMediaCategory, aNodeType, aCapability, aHasHardwareAssist, aQueryInterface)
                , iNodeCreationInterface(&aNodeCreation)
        {
            iNodeInstance = NULL;
        }


        /**
        ** Optional pointer to node creation implementation.  Nodes must register
        ** with either an instance pointer, or else a creation interface implementation.
        */
        PVMFNodeCreationInterface* iNodeCreationInterface;
        /**
        ** Optional pointer to node instance.  Nodes must register
        ** with either an instance pointer, or else a creation interface implementation.
        */
        PVMFNodeInterface* iNodeInstance;
};


/*
** A class containing all data required to register a media I/O component.
** The component may be registered with either an implemenation instance,
** or a creation interface instance.
*/
class PVMFMediaIORegistrationInfo: public PVMFComponentRegistrationInfo
{
    public:
        PVMFMediaIORegistrationInfo()
        {
            iMediaIOCreationInterface = NULL;
            iMediaIOInstance = NULL;
        }
        PVMFMediaIORegistrationInfo(const PVMFMediaIORegistrationInfo& a)
                : PVMFComponentRegistrationInfo(a)
                , iMediaIOCreationInterface(a.iMediaIOCreationInterface)
                , iMediaIOInstance(a.iMediaIOInstance)
        {}

        PVMFMediaIORegistrationInfo(PVUuid& aUuid
                                    , char* aMediaCategory
                                    , char* aMediaIOType
                                    , PVMFComponentCapability& aCapability
                                    , bool aHasHardwareAssist
                                    , PVMFSimpleQueryInterface* aQueryInterface
                                    , PvmiMIOControl& aMediaIOInstance)
                : PVMFComponentRegistrationInfo(aUuid, aMediaCategory, aMediaIOType, aCapability, aHasHardwareAssist, aQueryInterface)
                , iMediaIOInstance(&aMediaIOInstance)
        {
            iMediaIOCreationInterface = NULL;
        }

        PVMFMediaIORegistrationInfo(PVUuid& aUuid
                                    , char* aMediaCategory
                                    , char* aMediaIOType
                                    , PVMFComponentCapability& aCapability
                                    , bool aHasHardwareAssist
                                    , PVMFSimpleQueryInterface* aQueryInterface
                                    , PVMFMediaIOCreationInterface& aMediaIOCreation)
                : PVMFComponentRegistrationInfo(aUuid, aMediaCategory, aMediaIOType, aCapability, aHasHardwareAssist, aQueryInterface)
                , iMediaIOCreationInterface(&aMediaIOCreation)
        {
            iMediaIOInstance = NULL;
        }


        /**
        ** Optional pointer to Media I/O creation implementation.  Components must register
        ** with either an instance pointer, or else a creation interface implementation.
        */
        PVMFMediaIOCreationInterface* iMediaIOCreationInterface;
        /**
        ** Optional pointer to Media I/O control instance.  Components must register
        ** with either an instance pointer, or else a creation interface implementation.
        */
        PvmiMIOControl* iMediaIOInstance;
};


typedef uint32 PVMFRegistryIdType;

/*
** Base class for component registry entries.
*/
class PVMFComponentRegistryEntry
{
    public:
        //match category, type, and formats.
        bool Match(char* aMediaCategory = NULL
                                          , char* aComponentType = NULL
                                                                   , char* aInputFormat = NULL
                                                                                          , char* aOutputFormat = NULL);

        //match individual items.
        bool MatchMediaCategory(char* m);
        bool MatchComponentType(char* m);
        bool MatchFormat(char* in, char* out);
        bool MatchUuid(PVUuid&);

    protected:

        PVMFComponentRegistryEntry(PVMFComponentRegistrationInfo &aInfo
                                   , PVMFRegistryIdType aId)
                : iInfo(aInfo)
                , iId(aId)
        {}

        ~PVMFComponentRegistryEntry();

        //The component registration info.
        PVMFComponentRegistrationInfo& iInfo;

        //Component ID assigned during registration.
        PVMFRegistryIdType iId;

    private:
        static bool matchFormat(PVMFComponentFormatType&, PVMFComponentFormatListType&);
        static bool matchFormatPair(PVMFComponentFormatType&, PVMFComponentFormatType&, PVMFComponentFormatPairListType&);
        static bool matchInputFormat(PVMFComponentFormatType&, PVMFComponentFormatPairListType&);
        static bool matchOutputFormat(PVMFComponentFormatType&, PVMFComponentFormatPairListType&);

};


/*
** A class defining the node registry entries.
*/
class PVMFNodeRegistryEntry: public PVMFComponentRegistryEntry
{
    public:
        /*
        ** Methods to create and release a node instance
        ** for this entry.
        ** The Get and Release calls must be balanced.
        ** @param aParam: optional param to pass to CreateNode.
        */
        OSCL_IMPORT_REF PVMFNodeInterface* GetNodeInstance(OsclAny*aParam = NULL);
        OSCL_IMPORT_REF void ReleaseNodeInstance(PVMFNodeInterface*);

        /**
        ** For accessing the registration data
        */
        const PVMFNodeRegistrationInfo& Info()const
        {
            return iInfo;
        }

    private:
        friend class PVMFNodeRegistry;

        PVMFNodeRegistryEntry(PVMFNodeRegistrationInfo& aInfo
                              , PVMFRegistryIdType aId)
                : PVMFComponentRegistryEntry(iInfo, aId)
                , iInfo(aInfo)
        {}

        PVMFNodeRegistrationInfo iInfo;

};

/*
** A class defining the Media I/O registry entries.
*/
class PVMFMediaIORegistryEntry: public PVMFComponentRegistryEntry
{
    public:
        /*
        ** Methods to create and release a Media I/O instance
        ** for this entry.
        ** The Get and Release calls must be balanced.
        ** @param: optional param to pass to CreateMediaIO
        */
        OSCL_IMPORT_REF PvmiMIOControl* GetMediaIOInstance(OsclAny*aParam = NULL);
        OSCL_IMPORT_REF void ReleaseMediaIOInstance(PvmiMIOControl*);

        /**
        ** For accessing the registration data
        */
        const PVMFMediaIORegistrationInfo& Info()const
        {
            return iInfo;
        }

    private:
        friend class PVMFMediaIORegistry;

        PVMFMediaIORegistryEntry(PVMFMediaIORegistrationInfo& aInfo
                                 , PVMFRegistryIdType aId)
                : PVMFComponentRegistryEntry(iInfo, aId)
                , iInfo(aInfo)
        {}

        PVMFMediaIORegistrationInfo iInfo;

};


/*
** The node registry
*/


typedef	Oscl_Vector<PVMFNodeRegistryEntry*, OsclMemAllocator> PVMFNodeList;

class PVMFNodeRegistry
{
    public:
        OSCL_IMPORT_REF PVMFNodeRegistry();
        OSCL_IMPORT_REF ~PVMFNodeRegistry();

        /*
        ** Method to register a node.
        ** @param: the node entry.
        ** @return: the registry ID.
        */
        OSCL_IMPORT_REF PVMFRegistryIdType RegisterNode(PVMFNodeRegistrationInfo &entry);

        /*
        ** Method to unregister a node.
        ** @param: registry ID of the node
        */
        OSCL_IMPORT_REF void UnregisterNode(PVMFRegistryIdType);

        /*
        ** Method to unregister all nodes.
        */
        OSCL_IMPORT_REF void Clear();

        /*
        ** Method to find all entries that match a given criteria.
        ** Input parameters are used to filter the search.  Any
        ** input that is NULL will be ignored.
        ** @param aMatchList(output): list of components that satisfy the query.
        ** @param aMediaCategory(input): media category string
        ** @param aComponentType(input): media component type string
        ** @param aInputFormat(input): input format
        ** @param aOutputFormat(input): output format
        */
        OSCL_IMPORT_REF void Find(PVMFNodeList& aMatchList
                                  , char* aMediaCategory = NULL
                                                           , char* aComponentType = NULL
                                                                                    , char* aInputFormat = NULL
                                                                                                           , char* aOutputFormat = NULL);

        OSCL_IMPORT_REF void Find(PVMFNodeList& aMatchList
                                  , PVUuid& aUuid);

        /*
        ** For direct access to the node list
        */
        PVMFNodeList& NodeList()
        {
            return iNodeList;
        }

    private:
        PVMFRegistryIdType iIdCounter;

        PVMFNodeList iNodeList;

};

/*
** The media I/O registry
*/

typedef	Oscl_Vector<PVMFMediaIORegistryEntry*, OsclMemAllocator> PVMFMediaIOList;

class PVMFMediaIORegistry
{
    public:
        OSCL_IMPORT_REF PVMFMediaIORegistry();
        OSCL_IMPORT_REF ~PVMFMediaIORegistry();

        /*
        ** Method to register a Media I/O component
        ** @param: the media I/O component entry.
        ** @return: the registry ID.
        */
        OSCL_IMPORT_REF PVMFRegistryIdType RegisterMediaIO(PVMFMediaIORegistrationInfo& entry);

        /*
        ** Method to unregister a Media I/O component.
        ** @param: registry ID of the component
        */
        OSCL_IMPORT_REF void UnregisterMediaIO(PVMFRegistryIdType aId);

        /*
        ** Method to unregister all components.
        */
        OSCL_IMPORT_REF void Clear();

        /*
        ** Method to find all entries that match a given criteria.
        ** Input parameters are used to filter the search.  Any
        ** input that is NULL will be ignored.
        ** @param aMatchList(output): list of components that satisfy the query.
        ** @param aMediaCategory(input): media category string
        ** @param aComponentType(input): media component type string
        ** @param aInputFormat(input): input format
        ** @param aOutputFormat(input): output format
        */
        OSCL_IMPORT_REF void Find(PVMFMediaIOList& aMatchList
                                  , char* aMediaCategory = NULL
                                                           , char* aComponentType = NULL
                                                                                    , char* aInputFormat = NULL
                                                                                                           , char* aOutputFormat = NULL);

        OSCL_IMPORT_REF void Find(PVMFMediaIOList& aMatchList
                                  , PVUuid& aUuid);

        /*
        ** For direct access to the component list
        */
        PVMFMediaIOList& MediaIOList()
        {
            return iMediaIOList;
        }

    private:
        PVMFRegistryIdType iIdCounter;

        PVMFMediaIOList iMediaIOList;

};

#endif //PVMF_NODE_REGSITRY_H_INCLUDED




