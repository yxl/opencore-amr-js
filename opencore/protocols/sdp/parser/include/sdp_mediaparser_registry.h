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
#ifndef SDP_MEDIAPARSER_REGISTRY_H
#define SDP_MEDIAPARSER_REGISTRY_H
#include "oscl_mem.h"
#include "oscl_map.h"
#include "oscl_string_containers.h"
#include "oscl_base_alloc.h"
#include "oscl_string.h"
#include "oscl_str_ptr_len.h"

//use an appropriate registry
#include "oscl_error.h"
// TLS registry
#define SDPMEDIAPARSER_REGISTRY OsclTLSRegistryEx
#define SDPMEDIAPARSER_REGISTRY_ID OSCL_TLS_ID_SDPMEDIAPARSER
#define SDPMEDIAPARSER_REGISTRY_WRAPPER OsclTLSEx

class SDPBaseMediaInfoParser;

class SDPMediaParserFactory : public OsclDestructDealloc
{
    public:
        virtual ~SDPMediaParserFactory() {}
        virtual SDPBaseMediaInfoParser* createSDPMediaParserInstance() = 0;
        virtual void destruct_and_dealloc(OsclAny* ptr) = 0;
};

typedef OSCL_HeapString<OsclMemAllocator> string_key_type;

template<class Alloc> struct SDPRegMimeStringCompare
{
    bool operator()(const string_key_type& x, const string_key_type& y) const
    {
        if ((oscl_CIstrcmp(x.get_str(), y.get_str())) < 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
};

class SDPMediaParserRegistry
{
    public:

        /**
         * SDPMediaParserRegistry needs to be initialized once per thread.
         * This creates the SDPMediaParserRegistry singleton that is used
         * throughout the duration of the thread.
         *
         * @exception leaves if out of memory
         */
        static void Init()
        {
            /*
             * If the registry does not exist, create one
             */
            if (SDPMEDIAPARSER_REGISTRY::getInstance(SDPMEDIAPARSER_REGISTRY_ID) == NULL)
            {
                Oscl_TAlloc<SDPMediaParserRegistry, OsclMemAllocator> talloc;
                SDPMediaParserRegistry *sdpMediaParserReg = OSCL_ALLOC_NEW(talloc, SDPMediaParserRegistry, ());
                SDPMEDIAPARSER_REGISTRY::registerInstance(sdpMediaParserReg, SDPMEDIAPARSER_REGISTRY_ID);
            }
        }


        /**
         * Frees the SDPMediaParserRegistry singleton used by the current
         * thread. This must be called before thread exit.
         *
         * @return
         */
        static void Cleanup()
        {
            SDPMediaParserRegistry *sdpMediaParserReg =
                OSCL_STATIC_CAST(SDPMediaParserRegistry*, SDPMEDIAPARSER_REGISTRY::getInstance(SDPMEDIAPARSER_REGISTRY_ID));

            Oscl_TAlloc<SDPMediaParserRegistry, OsclMemAllocator> talloc;
            OSCL_ALLOC_DELETE(sdpMediaParserReg, talloc, SDPMediaParserRegistry);

            SDPMEDIAPARSER_REGISTRY::registerInstance(NULL, SDPMEDIAPARSER_REGISTRY_ID);
        }

        virtual ~SDPMediaParserRegistry() {};

        /**
         * Get the sdp media parser registry.  There is only one
         * registry instance per thread.
         */
        static SDPMediaParserRegistry* GetSDPMediaParserRegistry()
        {
            SDPMEDIAPARSER_REGISTRY_WRAPPER< SDPMediaParserRegistry, SDPMEDIAPARSER_REGISTRY_ID > sdpMediaParserRegSng;
            return &(*sdpMediaParserRegSng);
        }

        bool addMediaParserFactoryToRegistry(StrPtrLen mimeType,
                                             SDPMediaParserFactory* mediaParserFactory)
        {
            string_key_type mimeString(OSCL_CONST_CAST(const char*, mimeType.c_str()));
            _SDPMediaParserRegistry.insert(value_type(mimeString, mediaParserFactory));
            return true;
        };

        SDPMediaParserFactory*
        lookupSDPMediaParserFactory(OsclMemoryFragment memFrag)
        {
            string_key_type mimeString;
            mimeString.set((const char*)(memFrag.ptr), memFrag.len);
            Oscl_Map<string_key_type, SDPMediaParserFactory*, OsclMemAllocator, string_key_compare_class>::iterator it;
            it = _SDPMediaParserRegistry.find(mimeString);

            if (it != _SDPMediaParserRegistry.end())
            {
                return (((*it).second));
            }
            return NULL;
        }

    private:
        SDPMediaParserRegistry() {};

        typedef SDPRegMimeStringCompare<OsclMemAllocator> string_key_compare_class;
        typedef Oscl_Map<string_key_type, SDPMediaParserFactory*, OsclMemAllocator, string_key_compare_class>::value_type value_type;

        Oscl_Map<string_key_type, SDPMediaParserFactory*, OsclMemAllocator, string_key_compare_class> _SDPMediaParserRegistry;
};
#endif //SDP_REGISTRAR_H
