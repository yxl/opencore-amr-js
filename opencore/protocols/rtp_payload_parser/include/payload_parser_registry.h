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
#ifndef PAYLOAD_PARSER_REGISTRY_H_INCLUDED
#define PAYLOAD_PARSER_REGISTRY_H_INCLUDED


#include "oscl_mem.h"
#include "oscl_map.h"
#include "oscl_string_containers.h"
#include "oscl_base_alloc.h"
#include "oscl_string.h"
#include "oscl_str_ptr_len.h"

#include "payload_parser_factory.h"

//use the TLS registry, or the singleton registry if no TLS.
//Note: singleton registry only works in single-threaded scenarios, since this
//implementation assumes a per-thread registry.
#include "oscl_error.h"
#define PAYLOADPARSER_REGISTRY OsclTLSRegistryEx
#define PAYLOADPARSER_REGISTRY_ID OSCL_TLS_ID_PAYLOADPARSER
#define PAYLOADPARSER_REGISTRY_WRAPPER OsclTLSEx


typedef OSCL_HeapString<OsclMemAllocator> string_key_type;

class PayloadParserRegistry
{
    public:

        /**
         * PayloadParserRegistry needs to be initialized once per thread.
         * This creates the PayloadParserRegistry singleton that is used
         * throughout the duration of the thread.
         *
         * @exception leaves if out of memory
         */
        static void Init()
        {
            /*
             * If the registry does not exist, create one
             */
            if (PAYLOADPARSER_REGISTRY::getInstance(PAYLOADPARSER_REGISTRY_ID) == NULL)
            {
                Oscl_TAlloc<PayloadParserRegistry, OsclMemAllocator> talloc;
                PayloadParserRegistry *rtpPayloadParserReg =
                    OSCL_ALLOC_NEW(talloc, PayloadParserRegistry, ());
                PAYLOADPARSER_REGISTRY::registerInstance(rtpPayloadParserReg,
                        PAYLOADPARSER_REGISTRY_ID);
            }
        }


        /**am
         * Frees the PayloadParserRegistry singleton used by the current
         * thread. This must be called before thread exit.
         *
         * @return
         */
        static void Cleanup()
        {
            PayloadParserRegistry *rtpPayloadParserReg =
                OSCL_STATIC_CAST(PayloadParserRegistry*,
                                 PAYLOADPARSER_REGISTRY::getInstance(PAYLOADPARSER_REGISTRY_ID));

            Oscl_TAlloc<PayloadParserRegistry, OsclMemAllocator> talloc;
            OSCL_ALLOC_DELETE(rtpPayloadParserReg, talloc, PayloadParserRegistry);

            PAYLOADPARSER_REGISTRY::registerInstance(NULL, PAYLOADPARSER_REGISTRY_ID);
        }

        virtual ~PayloadParserRegistry() {};

        /**
         * Get the payload parser registry.  There is only one
         * registry instance per thread.
         */
        static PayloadParserRegistry* GetPayloadParserRegistry()
        {
            PAYLOADPARSER_REGISTRY_WRAPPER < PayloadParserRegistry,
            PAYLOADPARSER_REGISTRY_ID > payloadParserRegSng;
            return &(*payloadParserRegSng);
        }

        bool addPayloadParserFactoryToRegistry(StrPtrLen mimeType,
                                               IPayloadParserFactory* factory)
        {
            string_key_type mimeString(OSCL_CONST_CAST(const char*, mimeType.c_str()));
            iRegistry.insert(value_type(mimeString, factory));
            return true;
        };

        IPayloadParserFactory*
        lookupPayloadParserFactory(OsclMemoryFragment memFrag)
        {
            string_key_type mimeString;
            mimeString.set((char*)(memFrag.ptr), memFrag.len);
            Oscl_Map < string_key_type, IPayloadParserFactory*,
            OsclMemAllocator, MimeStringCompare >::iterator it;
            it = iRegistry.find(mimeString);

            /* This check is necessary for the ADS1.2 compiler*/
            if (!(it == iRegistry.end()))
            {
                return (((*it).second));
            }
            return NULL;
        }

        struct MimeStringCompare
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

    private:
        PayloadParserRegistry() {};

        typedef Oscl_Map < string_key_type, IPayloadParserFactory*, OsclMemAllocator,
        MimeStringCompare >::value_type value_type;

        Oscl_Map < string_key_type, IPayloadParserFactory*,
        OsclMemAllocator, MimeStringCompare > iRegistry;
};

#endif  // PAYLOAD_PARSER_REGISTRY_H_INCLUDED

