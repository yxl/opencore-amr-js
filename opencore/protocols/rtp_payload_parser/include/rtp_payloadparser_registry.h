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
#ifndef RTP_PAYLOADPARSER_REGISTRY_H
#define RTP_PAYLOADPARSER_REGISTRY_H


#include "oscl_mem.h"
#include "oscl_map.h"
#include "oscl_string_containers.h"
#include "oscl_base_alloc.h"
#include "oscl_string.h"
#include "oscl_str_ptr_len.h"
#include "rtp_payloadparser_factory_base.h"

//Need an appropriate registry
#include "oscl_error.h"
// TLS registry is available, use it
#define RTPPAYLOADPARSER_REGISTRY OsclTLSRegistryEx
#define RTPPAYLOADPARSER_REGISTRY_ID OSCL_TLS_ID_PAYLOADPARSER
#define RTPPAYLOADPARSER_REGISTRY_WRAPPER OsclTLSEx



typedef OSCL_HeapString<OsclMemAllocator> string_key_type;

template<class Alloc>
struct RTPPPMimeStringCompare
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

class RTPPayloadParserRegistry
{
    public:

        /**
         * RTPPayloadParserRegistry needs to be initialized once per thread.
         * This creates the RTPPayloadParserRegistry singleton that is used
         * throughout the duration of the thread.
         *
         * @exception leaves if out of memory
         */
        static void Init()
        {
            /*
             * If the registry does not exist, create one
             */
            if (RTPPAYLOADPARSER_REGISTRY::getInstance(RTPPAYLOADPARSER_REGISTRY_ID) == NULL)
            {
                Oscl_TAlloc<RTPPayloadParserRegistry, OsclMemAllocator> talloc;
                RTPPayloadParserRegistry *rtpPayloadParserReg = OSCL_ALLOC_NEW(talloc, RTPPayloadParserRegistry, ());
                RTPPAYLOADPARSER_REGISTRY::registerInstance(rtpPayloadParserReg, RTPPAYLOADPARSER_REGISTRY_ID);
            }
        }


        /**am
         * Frees the RTPPayloadParserRegistry singleton used by the current
         * thread. This must be called before thread exit.
         *
         * @return
         */
        static void Cleanup()
        {
            RTPPayloadParserRegistry *rtpPayloadParserReg =
                OSCL_STATIC_CAST(RTPPayloadParserRegistry*, RTPPAYLOADPARSER_REGISTRY::getInstance(RTPPAYLOADPARSER_REGISTRY_ID));

            Oscl_TAlloc<RTPPayloadParserRegistry, OsclMemAllocator> talloc;
            OSCL_ALLOC_DELETE(rtpPayloadParserReg, talloc, RTPPayloadParserRegistry);

            RTPPAYLOADPARSER_REGISTRY::registerInstance(NULL, RTPPAYLOADPARSER_REGISTRY_ID);
        }

        virtual ~RTPPayloadParserRegistry() {};

        /**
         * Get the rtp payload parser registry.  There is only one
         * registry instance per thread.
         */
        static RTPPayloadParserRegistry* GetRTPPayloadParserRegistry()
        {
            RTPPAYLOADPARSER_REGISTRY_WRAPPER< RTPPayloadParserRegistry, RTPPAYLOADPARSER_REGISTRY_ID > rtpPayloadParserRegSng;
            return &(*rtpPayloadParserRegSng);
        }

        bool addRTPPayloadParserFactoryToRegistry(StrPtrLen mimeType,
                RTPPayloadParserFactory* rtpPlayloadParserFactory)
        {
            string_key_type mimeString(OSCL_CONST_CAST(const char*, mimeType.c_str()));
            _RTPPayloadParserRegistry.insert(value_type(mimeString, rtpPlayloadParserFactory));
            return true;
        };

        RTPPayloadParserFactory*
        lookupRTPPayloadParserFactory(OsclMemoryFragment memFrag)
        {
            string_key_type mimeString;
            mimeString.set((char*)(memFrag.ptr), memFrag.len);
            Oscl_Map<string_key_type, RTPPayloadParserFactory*, OsclMemAllocator, string_key_compare_class>::iterator it;
            it = _RTPPayloadParserRegistry.find(mimeString);

            /* This convoluted check is necessary to satisfy the ADS1.2 compiler*/
            if (!(it == _RTPPayloadParserRegistry.end()))
            {
                return (((*it).second));
            }
            return NULL;
        }

    private:
        RTPPayloadParserRegistry() {};

        typedef RTPPPMimeStringCompare<OsclMemAllocator> string_key_compare_class;
        typedef Oscl_Map<string_key_type, RTPPayloadParserFactory*, OsclMemAllocator, string_key_compare_class>::value_type value_type;

        Oscl_Map<string_key_type, RTPPayloadParserFactory*, OsclMemAllocator, string_key_compare_class> _RTPPayloadParserRegistry;
};
#endif
