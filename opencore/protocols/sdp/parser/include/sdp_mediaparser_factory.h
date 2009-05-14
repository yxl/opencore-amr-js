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
#ifndef SDP_MEDIAPARSER_FACTORY_H
#define SDP_MEDIAPARSER_FACTORY_H
#include "oscl_types.h"
#include "sdp_mediaparser_registry.h"

class SDPAMRMediaParserFactory : public SDPMediaParserFactory
{
    public:
        virtual ~SDPAMRMediaParserFactory() {}
    private:
        virtual SDPBaseMediaInfoParser* createSDPMediaParserInstance();
        virtual void destruct_and_dealloc(OsclAny* ptr);
};

class SDPAACMediaParserFactory : public SDPMediaParserFactory
{
    public:
        virtual ~SDPAACMediaParserFactory() {}
    private:
        virtual SDPBaseMediaInfoParser* createSDPMediaParserInstance();
        virtual void destruct_and_dealloc(OsclAny* ptr);
};

class SDPH263MediaParserFactory : public SDPMediaParserFactory
{
    public:
        virtual ~SDPH263MediaParserFactory() {}
    private:
        virtual SDPBaseMediaInfoParser* createSDPMediaParserInstance();
        virtual void destruct_and_dealloc(OsclAny* ptr);
};

class SDPH264MediaParserFactory : public SDPMediaParserFactory
{
    public:
        virtual ~SDPH264MediaParserFactory() {}
    private:
        virtual SDPBaseMediaInfoParser* createSDPMediaParserInstance();
        virtual void destruct_and_dealloc(OsclAny* ptr);
};

class SDPMPEG4MediaParserFactory : public SDPMediaParserFactory
{
    public:
        virtual ~SDPMPEG4MediaParserFactory() {}
    private:
        virtual SDPBaseMediaInfoParser* createSDPMediaParserInstance();
        virtual void destruct_and_dealloc(OsclAny* ptr);
};

class SDPRFC3640MediaParserFactory : public SDPMediaParserFactory
{
        virtual SDPBaseMediaInfoParser* createSDPMediaParserInstance();
        virtual void destruct_and_dealloc(OsclAny* ptr);
};

class SDPStillImageMediaParserFactory : public SDPMediaParserFactory
{
    public:
        virtual ~SDPStillImageMediaParserFactory() {}
    private:
        virtual SDPBaseMediaInfoParser* createSDPMediaParserInstance();
        virtual void destruct_and_dealloc(OsclAny* ptr);
};

class SDPPCMAMediaParserFactory : public SDPMediaParserFactory
{
    public:
        virtual ~SDPPCMAMediaParserFactory() {}
    private:
        virtual SDPBaseMediaInfoParser* createSDPMediaParserInstance();
        virtual void destruct_and_dealloc(OsclAny* ptr);
};

class SDPPCMUMediaParserFactory : public SDPMediaParserFactory
{
    public:
        virtual ~SDPPCMUMediaParserFactory() {}
    private:
        virtual SDPBaseMediaInfoParser* createSDPMediaParserInstance();
        virtual void destruct_and_dealloc(OsclAny* ptr);
};

#endif //SDP_MEDIAPARSER_FACTORY_H
